package main

import (
	"fmt"
	"log"
	"os"
	"strings"
	"time"

	"github.com/gin-gonic/gin"
	"golang.org/x/sys/unix"
)

type SiodbLoggerPool struct {
	siodbLogger []*SiodbLogger
}

func (l *SiodbLoggerPool) ConfigGinLogger() error {

	file, err := os.Open("/dev/null")
	if err != nil {
		log.Fatal(err)
	}
	gin.DefaultWriter = file
	gin.DisableConsoleColor()
	gin.SetMode(gin.ReleaseMode)

	return nil

}

func (l *SiodbLoggerPool) GinFormattedOutput(param gin.LogFormatterParams) string {

	return FormattedOutput(INFO, "%s %s %s %s %d %s %s %s",
		param.ClientIP,
		param.Method,
		param.Path,
		param.Request.Proto,
		param.StatusCode,
		param.Latency,
		param.Request.UserAgent(),
		param.ErrorMessage,
	)
}

func (l *SiodbLoggerPool) Output(logLevel int, s string, v ...interface{}) error {

	for _, siodbLogger := range l.siodbLogger {
		siodbLogger.Output(logLevel, s, v...)
	}

	if logLevel == FATAL {
		os.Exit(2)
	}

	return nil

}

func FormattedOutput(logLevel int, s string, v ...interface{}) string {

	return fmt.Sprintf("%v %v %v %v %s\n",
		time.Now().UTC().Format("2006-01-02 15:04:05.999999"), ttos(logLevel), unix.Getpid(), unix.Gettid(),
		fmt.Sprintf(s, v...),
	)
}

func CreateSiodbLoggerPool(siodbConfigFile *SiodbConfigFile) (siodbLoggerPool SiodbLoggerPool, err error) {

	siodbLoggerPool = SiodbLoggerPool{}

	var channels []string
	if channels, err = siodbConfigFile.ParseLogChannelName(); err != nil {
		return siodbLoggerPool, err
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
				return siodbLoggerPool, fmt.Errorf("error for parameter 'log."+channel+".severity': %v", err)
			}
			if siodbLogger.severityLevel, err = StringToSeverityLevel(value); err != nil {
				return siodbLoggerPool, fmt.Errorf("error for parameter 'log."+channel+".severity': %v", err)
			}
			siodbLogger.logFileExpirationTimeout, _ = siodbConfigFile.GetParameterValue("log." + channel + ".exp_time")
			siodbLogger.maxLogFileSize, _ = siodbConfigFile.GetParameterValue("log." + channel + ".max_file_size")

		case "console":

			siodbLogger.channelType = CONSOLE
			siodbLogger.channelName = strings.ToLower(value)
			siodbLogger.destination, _ = siodbConfigFile.GetParameterValue("log." + channel + ".destination")
			if value, err = siodbConfigFile.GetParameterValue("log." + channel + ".severity"); err != nil {
				return siodbLoggerPool, fmt.Errorf("error for parameter 'log."+channel+".severity': %v", err)
			}
			if siodbLogger.severityLevel, err = StringToSeverityLevel(value); err != nil {
				return siodbLoggerPool, fmt.Errorf("error for parameter 'log."+channel+".severity': %v", err)
			}

		default:
			return siodbLoggerPool, fmt.Errorf("the channel tpye '%v' doesn't exist", strings.ToLower(value))
		}

		if err := siodbLogger.initLogger(); err != nil {
			return siodbLoggerPool, err
		}
		siodbLoggerPool.siodbLogger = append(siodbLoggerPool.siodbLogger, siodbLogger)

	}

	return siodbLoggerPool, nil

}
