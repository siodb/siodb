package main

import (
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"
)

func StringToByteSize(str string) (bytes uint64, err error) {

	if regexp.MustCompile(`^[a-zA-Z]+$`).MatchString(str[len(str)-1:]) {
		if bytes, err = strconv.ParseUint(str[:len(str)-1], 10, 64); err != nil {
			return 0, err
		}
		switch str[len(str)-1:] {
		case "k", "K":
			bytes = bytes * 1024
		case "m", "M":
			bytes = bytes * 1024 * 1024
		case "g", "G":
			bytes = bytes * 1024 * 1024 * 1024
		default:
			return 0, fmt.Errorf("Invalid unit for parameter '%v'", str)
		}
	} else {
		if bytes, err = strconv.ParseUint(str, 10, 64); err != nil {
			return 0, fmt.Errorf("Invalid parameter '%v'", str)
		}
	}

	return bytes, nil
}

func StringToSeconds(str string) (seconds uint64, err error) {

	if regexp.MustCompile(`^[a-zA-Z]+$`).MatchString(str[len(str)-1:]) {
		if seconds, err = strconv.ParseUint(str[:len(str)-1], 10, 64); err != nil {
			return 0, err
		}
		switch str[len(str)-1:] {
		case "M":
			seconds = seconds * 60
		case "h":
			seconds = seconds * 60 * 60
		case "d":
			seconds = seconds * 60 * 60 * 24
		case "W":
			seconds = seconds * 60 * 60 * 24 * 7
		case "m":
			seconds = seconds * 60 * 60 * 24 * 31
		case "y":
			seconds = seconds * 60 * 60 * 24 * 365
		default:
			return 0, fmt.Errorf("Invalid unit for parameter '%v'", str)
		}
	} else {
		if seconds, err = strconv.ParseUint(str, 10, 64); err != nil {
			return 0, fmt.Errorf("Invalid parameter '%v'", str)
		}
	}

	return seconds, nil
}

func StringToPortNumber(str string) (port uint32, err error) {
	var ui64 uint64
	if ui64, err = strconv.ParseUint(str, 10, 32); err != nil {
		return 0, fmt.Errorf("Invalid port number '%v'", str)
	}
	if ui64 < 0 || port > 65535 {
		return 0, fmt.Errorf("Invalid port number '%v'", str)
	}
	return uint32(ui64), nil
}

func StringToSeverityLevel(str string) (level uint, err error) {
	switch strings.ToLower(str) {
	case "trace":
		return 1, nil
	case "debug":
		return 2, nil
	case "info":
		return 3, nil
	case "warning":
		return 4, nil
	case "error":
		return 5, nil
	case "fatal":
		return 6, nil
	default:
		return 0, fmt.Errorf("Invalid severity level", str)
	}
	return level, nil
}

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

func verifyPath(SiodbInstanceConfigurationPath string, fileName string) (string, error) {

	if _, err := os.Stat(fileName); os.IsNotExist(err) {
		if _, err := os.Stat(filepath.Dir(SiodbInstanceConfigurationPath) + `/` + fileName); os.IsNotExist(err) {
			return "", fmt.Errorf("can't stat file %v", fileName)
		} else {
			return filepath.Dir(SiodbInstanceConfigurationPath) + `/` + fileName, nil
		}
	} else {
		return fileName, nil
	}
}

func AppendBytes(slice []byte, data []byte) []byte {
	m := len(slice)
	n := m + len(data)
	newSlice := make([]byte, n)
	copy(newSlice, slice)
	copy(newSlice[m:n], data)
	slice = newSlice
	return newSlice
}
