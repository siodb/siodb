// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

package main

import (
	"fmt"
	"io"
	"os"
	"strings"
	"sync"
	"time"

	"golang.org/x/sys/unix"
)

const (
	CONSOLE = 1
	FILE    = 2
)

const (
	TRACE   = 1
	DEBUG   = 2
	INFO    = 3
	WARNING = 4
	ERROR   = 5
	FATAL   = 6
)

type SiodbLogger struct {
	mutex                    sync.Mutex
	out                      io.Writer // logFile, os.Stdout
	file                     *os.File
	fileCreatedUnixTime      int64
	channelType              uint
	channelName              string
	destination              string
	maxLogFileSize           uint32
	logFileExpirationTimeout uint64 // second
	severityLevel            uint
}

func (logger *SiodbLogger) initLogger() (err error) {

	// Derive log destination
	switch strings.TrimSpace(strings.ToLower(logger.destination)) {
	case "stdout":
		logger.out = os.Stdout
	case "stderr":
		logger.out = os.Stderr
	default: // Assuming it's a path
		logger.createNewLogFile()
	}

	return nil
}

func (logger *SiodbLogger) closeLogFile() (err error) {
	if err = logger.file.Close(); err != nil {
		println(FormattedOutput(WARNING, "issue closing log file '%v': %v",
			logger.destination, err))
	}
	return nil
}

func (logger *SiodbLogger) createNewLogFile() {
	var err error
	logFileName := fmt.Sprintf("rest_%v_%v.log",
		time.Now().UTC().Format("20060102_150405"), unix.Getpid())

	var logFile *os.File
	if logFile, err = os.Create(
		strings.TrimSpace(strings.ToLower(logger.destination)) + "/" + logFileName); err != nil {
		siodbLoggerPool.FatalAndExit(FATAL_CANNOT_CREATE_LOG_FILE, "Cannot create log file: %v", err)
	}

	logger.fileCreatedUnixTime = time.Now().Unix()
	logger.file = logFile
	logger.out = logFile
}

func (logger *SiodbLogger) Output(logLevel int, log string) error {

	logger.mutex.Lock()
	defer logger.mutex.Unlock()

	//logger.logFileExpirationTimeout
	if logger.channelType == FILE && logger.logFileExpirationTimeout >= 0 {
		if time.Now().Unix() > logger.fileCreatedUnixTime+int64(logger.logFileExpirationTimeout) {
			logger.closeLogFile()
			logger.createNewLogFile()
		}
	}

	// Log
	if logLevel >= int(logger.severityLevel) {
		io.WriteString(logger.out, log)
	}

	// logger.maxLogFileSize
	if logger.channelType == FILE && logger.maxLogFileSize > 0 {
		if fileStat, err := logger.file.Stat(); err != nil {
			return err
		} else {
			if fileStat.Size() >= int64(logger.maxLogFileSize) {
				logger.closeLogFile()
				logger.createNewLogFile()
			}
		}
	}

	return nil
}
