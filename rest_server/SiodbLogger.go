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
	logChannelTypeConsole = 1
	logChannelTypeFile    = 2
)

const (
	logLevelTrace   = 1
	logLevelDebug   = 2
	logLevelInfo    = 3
	logLevelWarning = 4
	logLevelError   = 5
	logLevelFatal   = 6
)

type siodbLogger struct {
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

func (logger *siodbLogger) initLogger() (err error) {
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

func (logger *siodbLogger) closeLogFile() (err error) {
	if err = logger.file.Close(); err != nil {
		println(formattedOutput(logLevelWarning, "issue closing log file '%v': %v",
			logger.destination, err))
	}
	return nil
}

func (logger *siodbLogger) createNewLogFile() {
	var err error
	logFileName := fmt.Sprintf("rest_%v_%v.log",
		time.Now().UTC().Format("20060102_150405"), unix.Getpid())

	var logFile *os.File
	if logFile, err = os.Create(
		strings.TrimSpace(strings.ToLower(logger.destination)) + "/" + logFileName); err != nil {
		log.FatalAndExit(fatalCannotCreateLogFile, "Can't create log file: %v", err)
	}

	logger.fileCreatedUnixTime = time.Now().Unix()
	logger.file = logFile
	logger.out = logFile
}

func (logger *siodbLogger) Output(logLevel int, message string) error {
	logger.mutex.Lock()
	defer logger.mutex.Unlock()

	//logger.logFileExpirationTimeout
	if logger.channelType == logChannelTypeFile && logger.logFileExpirationTimeout > 0 {
		if time.Now().Unix() > logger.fileCreatedUnixTime+int64(logger.logFileExpirationTimeout) {
			logger.closeLogFile()
			logger.createNewLogFile()
		}
	}

	// Log
	if logLevel >= int(logger.severityLevel) {
		io.WriteString(logger.out, message)
	}

	// logger.maxLogFileSize
	if logger.channelType == logChannelTypeFile && logger.maxLogFileSize > 0 {
		fileStat, err := logger.file.Stat()
		if err != nil {
			return err
		}
		if fileStat.Size() >= int64(logger.maxLogFileSize) {
			logger.closeLogFile()
			logger.createNewLogFile()
		}
	}

	return nil
}
