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
	FATAL_INIT_ERROR                   = 2
	FATAL_UNABLE_TO_CLEANUP_TCP_BUFFER = 2
)

type SiodbLoggerPool struct {
	siodbLogger []*SiodbLogger
}

func (l *SiodbLoggerPool) ConfigGinLogger() {

	gin.SetMode(gin.ReleaseMode)
	gin.DisableConsoleColor()
	gin.DefaultWriter = ioutil.Discard

}

func (l *SiodbLoggerPool) LogRequest(c *gin.Context, latency time.Duration) {

	path := c.Request.URL.Path
	raw := c.Request.URL.RawQuery
	if raw != "" {
		path = path + "?" + raw
	}

	l.Output(INFO, "Request | %3d | %13v | %15s | %15v | %s | %-7s | %s",
		c.Writer.Status(),
		latency,
		c.ClientIP(),
		c.Request.UserAgent(),
		c.Request.Method,
		c.Request.Proto,
		path,
	)
}

func (l *SiodbLoggerPool) Output(logLevel int, s string, v ...interface{}) error {

	for _, siodbLogger := range l.siodbLogger {
		siodbLogger.Output(logLevel, s, v...)
	}

	return nil

}

func (l *SiodbLoggerPool) Trace(s string, v ...interface{}) {
	l.Output(TRACE, s, v...)
}

func (l *SiodbLoggerPool) Debug(s string, v ...interface{}) {
	l.Output(DEBUG, s, v...)
}

func (l *SiodbLoggerPool) Info(s string, v ...interface{}) {
	l.Output(INFO, s, v...)
}

func (l *SiodbLoggerPool) Warning(s string, v ...interface{}) {
	l.Output(WARNING, s, v...)
}

func (l *SiodbLoggerPool) Error(s string, v ...interface{}) {
	l.Output(ERROR, s, v...)
}

func (l *SiodbLoggerPool) Fatal(code int, s string, v ...interface{}) {
	l.Output(FATAL, s, v...)
	os.Exit(code)
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
