package main

import (
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

func StringToPortNumber(str string) (port uint32, err error) {
	var ui64 uint64
	if ui64, err = strconv.ParseUint(str, 10, 32); err != nil {
		return port, fmt.Errorf("cannot convert port string to port uint32: %v", str)
	}
	if ui64 < 0 || port > 65535 {
		return port, fmt.Errorf("port number out of standard port range", str)
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
		return 0, fmt.Errorf("severity level out of authorized range (1-6)", str)
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
			return "", fmt.Errorf("unable to verify file path for value '%v'", fileName)
		} else {
			return filepath.Dir(SiodbInstanceConfigurationPath) + `/` + fileName, nil
		}
	} else {
		return fileName, nil
	}
}
