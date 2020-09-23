package main

// specs from https://github.com/siodb/siodb/blob/master/docs/dev/designs/RestServer.md
// TODO: Protect IOMgrbuff with a limit to which it will start to stream
// TODO: Implement "basic" authentication as described in RestServer spec

import (
	"flag"
	"fmt"
	"os"
	"os/signal"
	"syscall"
	"time"

	"golang.org/x/sys/unix"
)

var (
	SiodbInstanceConfigurationRootPath string = "/etc/siodb/instances"
	restServerConfig                   RestServerConfig
	IOMgrCPool                         *IOMgrConnPool
	IOMgrCPoolMinConn                  = 1
	IOMgrCPoolMaxConn                  = 1
	siodbLoggerPool                    SiodbLoggerPool
)

func main() {

	// Parse Args
	SiodbInstanceName := flag.String("instance", "siodb", "Instance name")
	flag.Parse()

	// Signals
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGTERM)

	// Variables
	SiodbInstanceConfigurationPath := SiodbInstanceConfigurationRootPath + "/" + *SiodbInstanceName
	var err error

	// Parse Siodb instance parameters file
	siodbConfigFile := &SiodbConfigFile{}
	siodbConfigFile.path = SiodbInstanceConfigurationPath + "/config"
	if err := siodbConfigFile.ParseParameters(); err != nil {
		fmt.Printf("%v %v %v %v REST Server | %s\n", time.Now().UTC().Format("2006-01-02 15:04:05.999999"),
			ttos(FATAL), unix.Getpid(), unix.Gettid(), err)
		os.Exit(2)
	}

	// Logging setup
	if siodbLoggerPool, err = CreateSiodbLoggerPool(siodbConfigFile); err != nil {
		fmt.Printf("%v %v %v %v REST Server | %s\n", time.Now().UTC().Format("2006-01-02 15:04:05.999999"),
			ttos(FATAL), unix.Getpid(), unix.Gettid(), err)
		os.Exit(2)
	}
	siodbLoggerPool.ConfigGinLogger()
	siodbLoggerPool.Info("Logger Pool initialized successfully")

	// Create REST Server config
	if err = restServerConfig.ParseAndValidateConfiguration(siodbConfigFile); err != nil {
		siodbLoggerPool.Fatal(FATAL_INIT_ERROR, "cannot parse the rest configuration properly", err)
	}
	siodbLoggerPool.Info("REST server config initialized successfully")

	// Connection pool to IOMgr
	IOMgrCPool, err = CreateIOMgrConnPool(siodbConfigFile, IOMgrCPoolMinConn, IOMgrCPoolMaxConn)
	if err != nil {
		siodbLoggerPool.Fatal(FATAL_INIT_ERROR, "Cannot create connection pool: %v", err)
	}
	siodbLoggerPool.Info("IOMgr connection pool initialized successfully")

	// Start routers
	if restServerConfig.Ipv4HTTPPort != 0 {
		go func() {
			var restWorker RestWorker
			if err := restWorker.CreateRouter(restServerConfig.Ipv4HTTPPort); err != nil {
				siodbLoggerPool.Fatal(FATAL_INIT_ERROR, "Cannot create route: %v", err)
			}
			if err := restWorker.StartHTTPRouter(); err != nil {
				siodbLoggerPool.Fatal(FATAL_INIT_ERROR, "Cannot start router: %v", err)
			}
		}()
	}
	if restServerConfig.Ipv4HTTPSPort != 0 {
		go func() {
			var restWorker RestWorker
			if err := restWorker.CreateRouter(restServerConfig.Ipv4HTTPSPort); err != nil {
				siodbLoggerPool.Fatal(FATAL_INIT_ERROR, "Cannot create route: %v", err)
			}
			if err := restWorker.StartHTTPSRouter(restServerConfig.TLSCertificate, restServerConfig.TLSPrivateKey); err != nil {
				siodbLoggerPool.Fatal(FATAL_INIT_ERROR, "Cannot start router: %v", err)
			}
		}()
	}
	if restServerConfig.Ipv6HTTPPort != 0 {
		go func() {
			var restWorker RestWorker
			if err := restWorker.CreateRouter(restServerConfig.Ipv6HTTPPort); err != nil {
				siodbLoggerPool.Fatal(FATAL_INIT_ERROR, "Cannot create route: %v", err)
			}
			if err := restWorker.StartHTTPRouter(); err != nil {
				siodbLoggerPool.Fatal(FATAL_INIT_ERROR, "Cannot start router: %v", err)
			}
		}()
	}
	if restServerConfig.Ipv6HTTPSPort != 0 {
		go func() {
			var restWorker RestWorker
			if err := restWorker.CreateRouter(restServerConfig.Ipv6HTTPSPort); err != nil {
				siodbLoggerPool.Fatal(FATAL_INIT_ERROR, "Cannot create route: %v", err)
			}
			if err := restWorker.StartHTTPSRouter(restServerConfig.TLSCertificate, restServerConfig.TLSPrivateKey); err != nil {
				siodbLoggerPool.Fatal(FATAL_INIT_ERROR, "Cannot start router: %v", err)
			}
		}()
	}

	sig := <-sigs
	siodbLoggerPool.Info("Signal received: %v", sig)

}
