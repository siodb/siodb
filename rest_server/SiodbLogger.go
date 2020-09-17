package main

import (
	"fmt"
	"io"
	"log"
	"os"
	"strings"
	"sync"
	"testing"
	"time"

	"github.com/gin-gonic/gin"
	"golang.org/x/sys/unix"
)

const (
	TRACE   = 1
	DEBUG   = 2
	INFO    = 3
	WARNING = 4
	ERROR   = 5
	FATAL   = 6
)

func ttos(t int) string {
	switch t {
	case 1:
		return "trace"
	case 2:
		return "debug"
	case 3:
		return "info"
	case 4:
		return "warning"
	case 5:
		return "error"
	case 6:
		return "fatal"
	default:
		return "unknown"
	}
}

type SiodbLoggerPool struct {
	siodbLogger []SiodbLogger
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

func (l *SiodbLoggerPool) AddSiodbLogger(userLogLevel string, userLogDestination string) error {

	// Derive logLevel
	var logLevel int
	switch strings.TrimSpace(strings.ToUpper(userLogLevel)) {
	case "TRACE":
		logLevel = TRACE
	case "DEBUG":
		logLevel = DEBUG
	case "INFO":
		logLevel = INFO
	case "WARNING":
		logLevel = WARNING
	case "ERROR":
		logLevel = ERROR
	case "FATAL":
		logLevel = FATAL
	default:
		log.Fatalf("Unknown log level: %s.\n",
			strings.TrimSpace(strings.ToUpper(userLogLevel)))
	}

	// Derive log destination
	var logDestination io.Writer
	var err error
	switch strings.TrimSpace(strings.ToLower(userLogDestination)) {
	case "stdout":
		logDestination = os.Stdout
	case "stderr":
		logDestination = os.Stderr
	default: // Assuming it's a path

		logFileName := fmt.Sprintf("rest_%v_%v.log",
			time.Now().UTC().Format("20060102_150405"), unix.Getppid())
		if _, err := os.Stat(strings.TrimSpace(strings.ToLower(userLogDestination))); os.IsNotExist(err) {
			log.Fatalf("Unknown log destination: %s.\n",
				strings.TrimSpace(strings.ToLower(userLogDestination)))
		}
		if logDestination, err = os.Create(
			strings.TrimSpace(strings.ToLower(userLogDestination)) + "/" + logFileName); err != nil {
			log.Fatalf("Can't create log file: %v.\n", err)
		}

	}

	l.siodbLogger = append(l.siodbLogger,
		SiodbLogger{
			logLevel: logLevel,
			out:      logDestination,
		})

	return nil

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

type SiodbLogger struct {
	mu       sync.Mutex
	out      io.Writer // logFile, os.Stdout
	logLevel int
}

func (l *SiodbLogger) Output(logLevel int, s string, v ...interface{}) error {

	l.mu.Lock()
	defer l.mu.Unlock()

	if logLevel >= l.logLevel {
		io.WriteString(l.out, FormattedOutput(logLevel, s, v...))
	}

	return nil

}

func FormattedOutput(logLevel int, s string, v ...interface{}) string {

	return fmt.Sprintf("%v %v %v %v %s\n",
		time.Now().UTC().Format("2006-01-02 15:04:05.999999"), ttos(logLevel), unix.Getppid(), unix.Gettid(),
		fmt.Sprintf(s, v...),
	)
}

func TestSiodbLogger(*testing.T) {

	siodbLoggerPool := &SiodbLoggerPool{}
	siodbLoggerPool.AddSiodbLogger("info", "stdout")
	siodbLoggerPool.AddSiodbLogger("info", "/tmp")

	siodbLoggerPool.Output(TRACE, "This is TRACE being logged!")
	siodbLoggerPool.Output(DEBUG, "This is DEBUG being logged!")
	siodbLoggerPool.Output(INFO, "This is INFO being logged!")
	siodbLoggerPool.Output(WARNING, "This is WARNING being logged!")
	siodbLoggerPool.Output(ERROR, "This is ERROR being logged!")
	siodbLoggerPool.Output(FATAL, "This is FATAL being logged!")

}
