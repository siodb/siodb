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
	siodbInstanceConfigurationRootPath string = "/etc/siodb/instances"
	config                             restServerConfig
	ioMgrCPool                         *ioMgrConnPool
	ioMgrCPoolMinConn                  = 1
	ioMgrCPoolMaxConn                  = 8
	log                         *siodbLoggerPool
)

func main() {
	// Parse Args
	SiodbInstanceName := flag.String("instance", "", "Instance name")
	flag.Parse()
	if len(*SiodbInstanceName) == 0 {
		fmt.Printf("%v %v %v %v %s\n", time.Now().UTC().Format("2006-01-02 15:04:05.999999"),
			severityLevelToString(logLevelFatal), unix.Getpid(), unix.Gettid(), "Invalid instance name in argument '--instance'")
		os.Exit(1)
	}

	// Signals
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGTERM)

	// Variables
	siodbInstanceConfigurationPath := siodbInstanceConfigurationRootPath + "/" + *SiodbInstanceName
	var err error

	// Parse Siodb instance parameters file
	siodbConfigFile := &siodbConfigFile{}
	siodbConfigFile.path = siodbInstanceConfigurationPath + "/config"
	if err := siodbConfigFile.ParseParameters(); err != nil {
		fmt.Printf("%v %v %v %v %s\n", time.Now().UTC().Format("2006-01-02 15:04:05.999999"),
			severityLevelToString(logLevelFatal), unix.Getpid(), unix.Gettid(), err)
		os.Exit(2)
	}

	// Logging setup
	if log, err = createLog(siodbConfigFile); err != nil {
		fmt.Printf("%v %v %v %v %s\n", time.Now().UTC().Format("2006-01-02 15:04:05.999999"),
			severityLevelToString(logLevelFatal), unix.Getpid(), unix.Gettid(), err)
		os.Exit(2)
	}
	log.ConfigGinLogger()
	log.Info("Logging started")

	// Create REST Server config
	if err = config.ParseAndValidateConfiguration(siodbConfigFile); err != nil {
		log.FatalAndExit(fatalInitError, "Invalid REST Server configuration: %v", err)
	}
	log.Info("Siodb REST Server v.%v.%v.%v",
		siodbVersionMajor, siodbVersionMinor, siodbVersionPatch)
	log.Info("Copyright (C) %s Siodb GmbH. All rights reserved.", siodbCopyrightYears)

	// Connection pool to IOMgr
	ioMgrCPool, err = createIOMgrConnPool(siodbConfigFile, ioMgrCPoolMinConn, ioMgrCPoolMaxConn)
	if err != nil {
		log.FatalAndExit(fatalInitError, "Can't create connection pool: %v", err)
	}
	log.Info("IOMgr connection pool initialized successfully")

	// Start routers
	if config.ipv4HTTPPort != 0 {
		go func() {
			var worker restWorker
			if err := worker.CreateRouter(config.ipv4HTTPPort); err != nil {
				log.FatalAndExit(fatalInitError, "Can't create route: %v", err)
			}
			if err := worker.StartHTTPRouter(); err != nil {
				log.FatalAndExit(fatalInitError, "Can't start router: %v", err)
			}
		}()
	}
	if config.ipv4HTTPSPort != 0 {
		go func() {
			var worker restWorker
			if err := worker.CreateRouter(config.ipv4HTTPSPort); err != nil {
				log.FatalAndExit(fatalInitError, "Can't create route: %v", err)
			}
			if err := worker.StartHTTPSRouter(config.tlsCertificate, config.tlsPrivateKey); err != nil {
				log.FatalAndExit(fatalInitError, "Can't start router: %v", err)
			}
		}()
	}
	if config.ipv6HTTPPort != 0 {
		go func() {
			var worker restWorker
			if err := worker.CreateRouter(config.ipv6HTTPPort); err != nil {
				log.FatalAndExit(fatalInitError, "Can't create route: %v", err)
			}
			if err := worker.StartHTTPRouter(); err != nil {
				log.FatalAndExit(fatalInitError, "Can't start router: %v", err)
			}
		}()
	}
	if config.ipv6HTTPSPort != 0 {
		go func() {
			var worker restWorker
			if err := worker.CreateRouter(config.ipv6HTTPSPort); err != nil {
				log.FatalAndExit(fatalInitError, "Can't create route: %v", err)
			}
			if err := worker.StartHTTPSRouter(config.tlsCertificate, config.tlsPrivateKey); err != nil {
				log.FatalAndExit(fatalInitError, "Can't start router: %v", err)
			}
		}()
	}

	sig := <-sigs
	log.Info("%v signal received, terminating... ", sig)
	ioMgrCPool.CloseAllConnections()
	log.ClosePool()
}
