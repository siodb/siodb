// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

package main

import (
	"fmt"
	"io/ioutil"
	"os"
	"strings"
	"time"

	"github.com/gin-gonic/gin"
	"golang.org/x/sys/unix"
)

var (
	LogFileSizeMin              uint32 = 1 * 1024 * 1024
	LogFileExpirationTimeoutMin uint64 = 1 * 60
)

var (
	FATAL_INIT_ERROR             = 2
	FATAL_CANNOT_CREATE_LOG_FILE = 3
)

type SiodbLoggerPool struct {
	siodbLogger []*SiodbLogger
}

func (loggerPool *SiodbLoggerPool) ConfigGinLogger() {
	gin.SetMode(gin.ReleaseMode)
	gin.DisableConsoleColor()
	gin.DefaultWriter = ioutil.Discard
}

func (loggerPool *SiodbLoggerPool) LogRequest(c *gin.Context, latency time.Duration) {
	path := c.Request.URL.Path
	raw := c.Request.URL.RawQuery
	if raw != "" {
		path = path + "?" + raw
	}

	loggerPool.Output(INFO, "Request | %3d | %13v | %15s | %15v | %s | %-7s | %s",
		c.Writer.Status(),
		latency,
		c.ClientIP(),
		c.Request.UserAgent(),
		c.Request.Method,
		c.Request.Proto,
		path,
	)
}

func (loggerPool *SiodbLoggerPool) Output(logLevel int, s string, v ...interface{}) error {
	log := FormattedOutput(logLevel, s, v...)
	for _, siodbLogger := range loggerPool.siodbLogger {
		siodbLogger.Output(logLevel, log)
	}
	return nil
}

func (loggerPool *SiodbLoggerPool) Trace(s string, v ...interface{}) {
	loggerPool.Output(TRACE, s, v...)
}

func (loggerPool *SiodbLoggerPool) Debug(s string, v ...interface{}) {
	loggerPool.Output(DEBUG, s, v...)
}

func (loggerPool *SiodbLoggerPool) Info(s string, v ...interface{}) {
	loggerPool.Output(INFO, s, v...)
}

func (loggerPool *SiodbLoggerPool) Warning(s string, v ...interface{}) {
	loggerPool.Output(WARNING, s, v...)
}

func (loggerPool *SiodbLoggerPool) Error(s string, v ...interface{}) {
	loggerPool.Output(ERROR, s, v...)
}

func (loggerPool *SiodbLoggerPool) FatalAndExit(code int, s string, v ...interface{}) {
	if code == FATAL_CANNOT_CREATE_LOG_FILE {
		println(FormattedOutput(FATAL, s, v...))
	} else {
		loggerPool.Output(FATAL, s, v...)
	}
	loggerPool.ClosePool()
	os.Exit(code)
}

func FormattedOutput(logLevel int, s string, v ...interface{}) string {
	return fmt.Sprintf("%v %v %v %v %s\n",
		time.Now().UTC().Format("2006-01-02 15:04:05.999999"),
		SeverityLevelToString(logLevel), unix.Getpid(), unix.Gettid(),
		fmt.Sprintf(s, v...),
	)
}

func (loggerPool *SiodbLoggerPool) ClosePool() {
	for _, siodbLogger := range loggerPool.siodbLogger {
		if siodbLogger.channelType != CONSOLE {
			siodbLogger.Output(INFO, FormattedOutput(INFO, "Logging stopped."))
			siodbLogger.closeLogFile()
		}
	}
	fmt.Println(FormattedOutput(INFO, "Logging stopped."))
}

func CreateSiodbLoggerPool(siodbConfigFile *SiodbConfigFile) (siodbLoggerPool *SiodbLoggerPool, err error) {

	siodbLoggerPool = &SiodbLoggerPool{}

	var channels []string
	if channels, err = siodbConfigFile.ParseLogChannelsName(); err != nil {
		return nil, err
	}
	for _, channel := range channels {

		value, err := siodbConfigFile.GetParameterValue("log." + channel + ".type")

		siodbLogger := &SiodbLogger{}

		switch strings.ToLower(value) {

		case "file":

			siodbLogger.channelType = FILE
			siodbLogger.channelName = strings.ToLower(value)
			siodbLogger.destination, _ = siodbConfigFile.GetParameterValue("log." + channel + ".destination")
			if value, err = siodbConfigFile.GetParameterValue("log." + channel + ".severity"); err != nil {
				return nil, fmt.Errorf("Invalid parameter 'log.%s.severity': %v", channel, err)
			}
			if siodbLogger.severityLevel, err = StringToSeverityLevel(value); err != nil {
				return nil, fmt.Errorf("Invalid parameter 'log.%s.severity': %v", channel, err)
			}
			if value, err = siodbConfigFile.GetParameterValue("log." + channel + ".max_file_size"); err != nil {
				siodbLogger.maxLogFileSize = 0
			} else {
				if siodbLogger.maxLogFileSize, err = StringToByteSize(value); err != nil {
					return nil, fmt.Errorf("Invalid value for parameter 'log.%s.max_file_size': %v", channel, err)
				}
				if siodbLogger.maxLogFileSize > 0 && siodbLogger.maxLogFileSize < LogFileSizeMin {
					return nil, fmt.Errorf("Invalid value for parameter 'log.%s.max_file_size': %v, expecting > %v",
						channel, siodbLogger.maxLogFileSize, LogFileSizeMin)
				}
			}
			if value, err = siodbConfigFile.GetParameterValue("log." + channel + ".exp_time"); err != nil {
				siodbLogger.logFileExpirationTimeout = 0
			} else {
				if siodbLogger.logFileExpirationTimeout, err = StringToSeconds(value); err != nil {
					return nil, fmt.Errorf("Invalid value for parameter 'log.%s.exp_time': %v", channel, err)
				}
				if siodbLogger.logFileExpirationTimeout > 0 && siodbLogger.logFileExpirationTimeout < LogFileExpirationTimeoutMin {
					return nil, fmt.Errorf("Invalid value for parameter 'log.%s.max_file_size': %v, expecting > %v",
						channel, siodbLogger.logFileExpirationTimeout, LogFileExpirationTimeoutMin)
				}
			}

		case "console":

			siodbLogger.channelType = CONSOLE
			siodbLogger.channelName = strings.ToLower(value)
			siodbLogger.destination, _ = siodbConfigFile.GetParameterValue("log." + channel + ".destination")
			if value, err = siodbConfigFile.GetParameterValue("log." + channel + ".severity"); err != nil {
				return nil, fmt.Errorf("Invalid parameter 'log.%s.severity': %v", channel, err)
			}
			if siodbLogger.severityLevel, err = StringToSeverityLevel(value); err != nil {
				return nil, fmt.Errorf("Invalid parameter 'log.%s.severity': %v", channel, err)
			}

		default:
			return nil, fmt.Errorf("Invalid channel type '%v'", strings.ToLower(value))
		}

		if err := siodbLogger.initLogger(); err != nil {
			return nil, err
		}
		siodbLoggerPool.siodbLogger = append(siodbLoggerPool.siodbLogger, siodbLogger)

	}

	return siodbLoggerPool, nil
}
