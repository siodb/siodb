// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// specs from https://github.com/siodb/siodb/blob/master/docs/dev/designs/RestServer.md

package main

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
	IOMgrCPoolMaxConn                  = 8
	siodbLoggerPool                    *SiodbLoggerPool
)

func main() {

	// Parse Args
	SiodbInstanceName := flag.String("instance", "", "Instance name")
	flag.Parse()
	if len(*SiodbInstanceName) == 0 {
		fmt.Printf("%v %v %v %v %s\n", time.Now().UTC().Format("2006-01-02 15:04:05.999999"),
			SeverityLevelToString(FATAL), unix.Getpid(), unix.Gettid(), "Invalid instance name in argument '--instance'")
		os.Exit(1)
	}

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
		fmt.Printf("%v %v %v %v %s\n", time.Now().UTC().Format("2006-01-02 15:04:05.999999"),
			SeverityLevelToString(FATAL), unix.Getpid(), unix.Gettid(), err)
		os.Exit(2)
	}

	// Logging setup
	if siodbLoggerPool, err = CreateSiodbLoggerPool(siodbConfigFile); err != nil {
		fmt.Printf("%v %v %v %v %s\n", time.Now().UTC().Format("2006-01-02 15:04:05.999999"),
			SeverityLevelToString(FATAL), unix.Getpid(), unix.Gettid(), err)
		os.Exit(2)
	}
	siodbLoggerPool.ConfigGinLogger()
	siodbLoggerPool.Info("Logging started")

	// Create REST Server config
	if err = restServerConfig.ParseAndValidateConfiguration(siodbConfigFile); err != nil {
		siodbLoggerPool.FatalAndExit(FATAL_INIT_ERROR, "Invalid REST Server configuration: %v", err)
	}
	siodbLoggerPool.Info("Siodb REST Server v.%v.%v.%v",
		SIODB_VERSION_MAJOR, SIODB_VERSION_MINOR, SIODB_VERSION_PATCH)
	siodbLoggerPool.Info("Copyright (C) %s Siodb GmbH. All rights reserved.", SIODB_COPYRIGHT_YEARS)

	// Connection pool to IOMgr
	IOMgrCPool, err = CreateIOMgrConnPool(siodbConfigFile, IOMgrCPoolMinConn, IOMgrCPoolMaxConn)
	if err != nil {
		siodbLoggerPool.FatalAndExit(FATAL_INIT_ERROR, "Can't create connection pool: %v", err)
	}
	siodbLoggerPool.Info("IOMgr connection pool initialized successfully")

	// Start routers
	if restServerConfig.Ipv4HTTPPort != 0 {
		go func() {
			var restWorker RestWorker
			if err := restWorker.CreateRouter(restServerConfig.Ipv4HTTPPort); err != nil {
				siodbLoggerPool.FatalAndExit(FATAL_INIT_ERROR, "Can't create route: %v", err)
			}
			if err := restWorker.StartHTTPRouter(); err != nil {
				siodbLoggerPool.FatalAndExit(FATAL_INIT_ERROR, "Can't start router: %v", err)
			}
		}()
	}
	if restServerConfig.Ipv4HTTPSPort != 0 {
		go func() {
			var restWorker RestWorker
			if err := restWorker.CreateRouter(restServerConfig.Ipv4HTTPSPort); err != nil {
				siodbLoggerPool.FatalAndExit(FATAL_INIT_ERROR, "Can't create route: %v", err)
			}
			if err := restWorker.StartHTTPSRouter(restServerConfig.TLSCertificate, restServerConfig.TLSPrivateKey); err != nil {
				siodbLoggerPool.FatalAndExit(FATAL_INIT_ERROR, "Can't start router: %v", err)
			}
		}()
	}
	if restServerConfig.Ipv6HTTPPort != 0 {
		go func() {
			var restWorker RestWorker
			if err := restWorker.CreateRouter(restServerConfig.Ipv6HTTPPort); err != nil {
				siodbLoggerPool.FatalAndExit(FATAL_INIT_ERROR, "Can't create route: %v", err)
			}
			if err := restWorker.StartHTTPRouter(); err != nil {
				siodbLoggerPool.FatalAndExit(FATAL_INIT_ERROR, "Can't start router: %v", err)
			}
		}()
	}
	if restServerConfig.Ipv6HTTPSPort != 0 {
		go func() {
			var restWorker RestWorker
			if err := restWorker.CreateRouter(restServerConfig.Ipv6HTTPSPort); err != nil {
				siodbLoggerPool.FatalAndExit(FATAL_INIT_ERROR, "Can't create route: %v", err)
			}
			if err := restWorker.StartHTTPSRouter(restServerConfig.TLSCertificate, restServerConfig.TLSPrivateKey); err != nil {
				siodbLoggerPool.FatalAndExit(FATAL_INIT_ERROR, "Can't start router: %v", err)
			}
		}()
	}

	sig := <-sigs
	siodbLoggerPool.Info("%v signal received, terminating... ", sig)
	IOMgrCPool.CloseAllConnections()
	siodbLoggerPool.ClosePool()
}
