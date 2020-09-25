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
	channelType              uint
	channelName              string
	destination              string
	maxLogFileSize           uint64
	logFileExpirationTimeout uint64 // second
	severityLevel            uint
}

func (logger *SiodbLogger) initLogger() error {

	// Derive log destination
	var err error
	switch strings.TrimSpace(strings.ToLower(logger.destination)) {
	case "stdout":
		logger.out = os.Stdout
	case "stderr":
		logger.out = os.Stderr
	default: // Assuming it's a path

		logFileName := fmt.Sprintf("rest_%v_%v.log",
			time.Now().UTC().Format("20060102_150405"), unix.Getpid())
		if _, err := os.Stat(strings.TrimSpace(strings.ToLower(logger.destination))); os.IsNotExist(err) {
			return fmt.Errorf("Invalid log destination: %s", strings.TrimSpace(strings.ToLower(logger.destination)))
		}

		var logFile io.Writer
		if logFile, err = os.Create(
			strings.TrimSpace(strings.ToLower(logger.destination)) + "/" + logFileName); err != nil {
			return fmt.Errorf("Invalid log file destination: %v", err)
		}
		logger.out = logFile

	}

	return nil
}

func (logger *SiodbLogger) Output(logLevel int, s string, v ...interface{}) error {

	logger.mutex.Lock()
	defer logger.mutex.Unlock()

	if logLevel >= int(logger.severityLevel) {
		io.WriteString(logger.out, FormattedOutput(logLevel, s, v...))
	}

	return nil
}
