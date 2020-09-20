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
	mu                       sync.Mutex
	out                      io.Writer // logFile, os.Stdout
	channelType              uint
	channelName              string
	destination              string
	maxLogFileSize           string
	logFileExpirationTimeout string
	severityLevel            uint
}

func (sl *SiodbLogger) initLogger() error {

	// Derive log destination
	var err error
	switch strings.TrimSpace(strings.ToLower(sl.destination)) {
	case "stdout":
		sl.out = os.Stdout
	case "stderr":
		sl.out = os.Stderr
	default: // Assuming it's a path

		logFileName := fmt.Sprintf("rest_%v_%v.log",
			time.Now().UTC().Format("20060102_150405"), unix.Getppid())
		if _, err := os.Stat(strings.TrimSpace(strings.ToLower(sl.destination))); os.IsNotExist(err) {
			return fmt.Errorf("Unknown log destination: %s", strings.TrimSpace(strings.ToLower(sl.destination)))
		}

		var logFile io.Writer
		if logFile, err = os.Create(
			strings.TrimSpace(strings.ToLower(sl.destination)) + "/" + logFileName); err != nil {
			return fmt.Errorf("can't create log file: %v", err)
		}
		sl.out = logFile

	}

	return nil

}

func (sl *SiodbLogger) Output(logLevel int, s string, v ...interface{}) error {

	sl.mu.Lock()
	defer sl.mu.Unlock()

	if logLevel >= int(sl.severityLevel) {
		io.WriteString(sl.out, FormattedOutput(logLevel, s, v...))
	}

	return nil

}
