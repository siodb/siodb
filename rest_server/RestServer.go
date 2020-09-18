package main

// specs from https://github.com/siodb/siodb/blob/master/docs/dev/designs/RestServer.md
// TODO: Implement "basic" authentication as describe in RestServer spec
// TODO: Parse parameters into a structures
// TODO: Protect IOMgrbuff with a limit to which it will start to stream
// TODO: log exit FATAL with code
// TODO: Finish the POST and DELETE implementation
// TODO: Put chunked WritePayload in function
// TODO: Put read chunked payload from IOMgr in function
// TODO: log format
// TODO: Once cxxman code step 5 in https://github.com/siodb/siodb/blob/master/docs/dev/designs/RestServer.md#post-patch-requests

import (
	"flag"
	"log"
	"os"
	"os/signal"
	"syscall"
)

type RestServerConfig struct {
	Ipv4HTTPPort          uint64
	Ipv4HTTPSPort         uint64
	Ipv6HTTPPort          uint64
	Ipv6HTTPSPort         uint64
	TLSCertificate        string
	TLSPrivateKey         string
	HTTPChunkSize         uint64
	logFileDestination    string
	logFileSeverity       string
	logConsoleDestination string
	logConsoleSeverity    string
}

var (
	SiodbInstanceConfigurationRootPath string = "/etc/siodb/instances"
	restServerConfig                   RestServerConfig
	IOMgrCPool                         *IOMgrConnPool
	siodbLoggerPool                    SiodbLoggerPool
)

var (
	DATABASEENGINERESPONSE    uint64 = 4
	DATABASEENGINERESTREQUEST uint64 = 13
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
	var config Config

	// Parse Siodb instance parameters file
	if config, err = parseInitFile(SiodbInstanceConfigurationPath + "/config"); err != nil {
		log.Fatalln(err)
	}

	// Validate and load parameters
	if err = restServerConfig.parseConfiguration(SiodbInstanceConfigurationPath, config); err != nil {
		log.Fatalln(err)
	}

	// Logging setup
	siodbLoggerPool.AddSiodbLogger(restServerConfig.logConsoleSeverity, restServerConfig.logConsoleDestination)
	siodbLoggerPool.AddSiodbLogger(restServerConfig.logFileSeverity, restServerConfig.logFileDestination)

	siodbLoggerPool.Output(INFO, "Starting REST server with config: %v", restServerConfig)

	// Gin Log at global level
	siodbLoggerPool.ConfigGinLogger()

	// Connection pool to IOMgr
	IOMgrCPool, err = CreateIOMgrConnPool(4, 8, config)
	if err != nil {
		siodbLoggerPool.Output(FATAL, "Cannot create connection pool: %v", err)
	}

	// Start routers
	if restServerConfig.Ipv4HTTPPort != 0 {
		go func() {
			var restWorker RestWorker
			if err := restWorker.CreateRouter(restServerConfig.Ipv4HTTPPort); err != nil {
				siodbLoggerPool.Output(FATAL, "Cannot create route: %v", err)
			}
			if err := restWorker.StartHTTPRouter(); err != nil {
				siodbLoggerPool.Output(FATAL, "Cannot start router: %v", err)
			}
		}()

	}
	if restServerConfig.Ipv4HTTPSPort != 0 {
		go func() {
			var restWorker RestWorker
			if err := restWorker.CreateRouter(restServerConfig.Ipv4HTTPSPort); err != nil {
				siodbLoggerPool.Output(FATAL, "Cannot create route: %v", err)
			}
			if err := restWorker.StartHTTPSRouter(restServerConfig.TLSCertificate, restServerConfig.TLSPrivateKey); err != nil {
				siodbLoggerPool.Output(FATAL, "Cannot start router: %v", err)
			}
		}()
	}
	if restServerConfig.Ipv6HTTPPort != 0 {
		go func() {
			var restWorker RestWorker
			if err := restWorker.CreateRouter(restServerConfig.Ipv6HTTPPort); err != nil {
				siodbLoggerPool.Output(FATAL, "Cannot create route: %v", err)
			}
			if err := restWorker.StartHTTPRouter(); err != nil {
				siodbLoggerPool.Output(FATAL, "Cannot start router: %v", err)
			}
		}()
	}
	if restServerConfig.Ipv6HTTPSPort != 0 {
		go func() {
			var restWorker RestWorker
			if err := restWorker.CreateRouter(restServerConfig.Ipv6HTTPSPort); err != nil {
				siodbLoggerPool.Output(FATAL, "Cannot create route: %v", err)
			}
			if err := restWorker.StartHTTPSRouter(restServerConfig.TLSCertificate, restServerConfig.TLSPrivateKey); err != nil {
				siodbLoggerPool.Output(FATAL, "Cannot start router: %v", err)
			}
		}()
	}

	sig := <-sigs
	siodbLoggerPool.Output(INFO, "Signal received: %v", sig)

}
