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
	logFileSizeMin              uint32 = 1 * 1024 * 1024
	logFileExpirationTimeoutMin uint64 = 1 * 60
)

var (
	fatalInitError           = 2
	fatalCannotCreateLogFile = 3
)

type siodbLoggerPool struct {
	loggers []*siodbLogger
}

func (log *siodbLoggerPool) ConfigGinLogger() {
	gin.SetMode(gin.ReleaseMode)
	gin.DisableConsoleColor()
	gin.DefaultWriter = ioutil.Discard
}

func (log *siodbLoggerPool) LogRequest(c *gin.Context, latency time.Duration) {
	path := c.Request.URL.Path
	raw := c.Request.URL.RawQuery
	if raw != "" {
		path = path + "?" + raw
	}

	log.Output(logLevelInfo, "Request | %3d | %13v | %15s | %15v | %s | %-7s | %s",
		c.Writer.Status(),
		latency,
		c.ClientIP(),
		c.Request.UserAgent(),
		c.Request.Method,
		c.Request.Proto,
		path,
	)
}

func (log *siodbLoggerPool) Output(logLevel int, s string, v ...interface{}) error {
	message := formattedOutput(logLevel, s, v...)
	for _, logger := range log.loggers {
		logger.Output(logLevel, message)
	}
	return nil
}

func (log *siodbLoggerPool) Trace(s string, v ...interface{}) {
	log.Output(logLevelTrace, s, v...)
}

func (log *siodbLoggerPool) Debug(s string, v ...interface{}) {
	log.Output(logLevelDebug, s, v...)
}

func (log *siodbLoggerPool) Info(s string, v ...interface{}) {
	log.Output(logLevelInfo, s, v...)
}

func (log *siodbLoggerPool) Warning(s string, v ...interface{}) {
	log.Output(logLevelWarning, s, v...)
}

func (log *siodbLoggerPool) Error(s string, v ...interface{}) {
	log.Output(logLevelError, s, v...)
}

func (log *siodbLoggerPool) FatalAndExit(code int, s string, v ...interface{}) {
	if code == fatalCannotCreateLogFile {
		println(formattedOutput(logLevelFatal, s, v...))
	} else {
		log.Output(logLevelFatal, s, v...)
	}
	log.ClosePool()
	os.Exit(code)
}

func formattedOutput(logLevel int, s string, v ...interface{}) string {
	return fmt.Sprintf("%v %v %v %v %s\n",
		time.Now().UTC().Format("2006-01-02 15:04:05.999999"),
		severityLevelToString(logLevel), unix.Getpid(), unix.Gettid(),
		fmt.Sprintf(s, v...),
	)
}

func (log *siodbLoggerPool) ClosePool() {
	for _, logger := range log.loggers {
		if logger.channelType != logChannelTypeConsole {
			logger.Output(logLevelInfo, formattedOutput(logLevelInfo, "Logging stopped."))
			logger.closeLogFile()
		}
	}
	fmt.Println(formattedOutput(logLevelInfo, "Logging stopped."))
}

func createLog(configFile *siodbConfigFile) (log *siodbLoggerPool, err error) {
	log = &siodbLoggerPool{}

	var channels []string
	if channels, err = configFile.ParseLogChannelsName(); err != nil {
		return nil, err
	}

	for _, channel := range channels {
		value, _ := configFile.GetParameterValue("log." + channel + ".type")
		logger := &siodbLogger{}

		switch strings.ToLower(value) {
		case "file":
			logger.channelType = logChannelTypeFile
			logger.channelName = strings.ToLower(value)
			logger.destination, _ = configFile.GetParameterValue("log." + channel + ".destination")
			if value, err = configFile.GetParameterValue("log." + channel + ".severity"); err != nil {
				return nil, fmt.Errorf("invalid parameter 'log.%s.severity': %v", channel, err)
			}
			if logger.severityLevel, err = stringToSeverityLevel(value); err != nil {
				return nil, fmt.Errorf("invalid parameter 'log.%s.severity': %v", channel, err)
			}
			if value, err = configFile.GetParameterValue("log." + channel + ".max_file_size"); err != nil {
				logger.maxLogFileSize = 0
			} else {
				if logger.maxLogFileSize, err = stringToByteSize(value); err != nil {
					return nil, fmt.Errorf("invalid value for parameter 'log.%s.max_file_size': %v", channel, err)
				}
				if logger.maxLogFileSize > 0 && logger.maxLogFileSize < logFileSizeMin {
					return nil, fmt.Errorf("invalid value for parameter 'log.%s.max_file_size': %v, expecting > %v",
						channel, logger.maxLogFileSize, logFileSizeMin)
				}
			}
			if value, err = configFile.GetParameterValue("log." + channel + ".exp_time"); err != nil {
				logger.logFileExpirationTimeout = 0
			} else {
				if logger.logFileExpirationTimeout, err = stringToSeconds(value); err != nil {
					return nil, fmt.Errorf("invalid value for parameter 'log.%s.exp_time': %v", channel, err)
				}
				if logger.logFileExpirationTimeout > 0 && logger.logFileExpirationTimeout < logFileExpirationTimeoutMin {
					return nil, fmt.Errorf("invalid value for parameter 'log.%s.max_file_size': %v, expecting > %v",
						channel, logger.logFileExpirationTimeout, logFileExpirationTimeoutMin)
				}
			}

		case "console":
			logger.channelType = logChannelTypeConsole
			logger.channelName = strings.ToLower(value)
			logger.destination, _ = configFile.GetParameterValue("log." + channel + ".destination")
			if value, err = configFile.GetParameterValue("log." + channel + ".severity"); err != nil {
				return nil, fmt.Errorf("invalid parameter 'log.%s.severity': %v", channel, err)
			}
			if logger.severityLevel, err = stringToSeverityLevel(value); err != nil {
				return nil, fmt.Errorf("invalid parameter 'log.%s.severity': %v", channel, err)
			}

		default:
			return nil, fmt.Errorf("invalid channel type '%v'", strings.ToLower(value))
		}

		if err := logger.initLogger(); err != nil {
			return nil, err
		}
		log.loggers = append(log.loggers, logger)
	}

	log.Info("Logging started")
	log.Debug("Created %v loggers", len(log.loggers))

	return log, nil
}
