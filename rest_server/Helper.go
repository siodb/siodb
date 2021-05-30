// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

package main

import (
	"fmt"
	"math"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

func stringToByteSize(str string) (bytes uint32, err error) {
	var mult uint64 = 1
	if strings.HasSuffix(str, "k") || strings.HasSuffix(str, "K") {
		mult = 1024
	} else if strings.HasSuffix(str, "m") || strings.HasSuffix(str, "M") {
		mult = 1024 * 1024
	} else if strings.HasSuffix(str, "g") || strings.HasSuffix(str, "G") {
		mult = 1024 * 1024 * 1024
	}

	var value uint64
	if mult > 1 {
		if value, err = strconv.ParseUint(str[:len(str)-1], 10, 32); err != nil {
			return 0, err
		}
	} else {
		if value, err = strconv.ParseUint(str, 10, 32); err != nil {
			return 0, err
		}
	}

	var upperBound = math.MaxUint32 / mult
	if value > upperBound {
		return 0, fmt.Errorf("number of bytes is too big: '%v'", str)
	}

	return uint32(value * mult), nil
}

func stringToSeconds(str string) (seconds uint64, err error) {
	var mult uint64 = 1
	if strings.HasSuffix(str, "m") || strings.HasSuffix(str, "M") {
		mult = 60
	} else if strings.HasSuffix(str, "h") || strings.HasSuffix(str, "H") {
		mult = 60 * 60
	} else if strings.HasSuffix(str, "d") || strings.HasSuffix(str, "D") {
		mult = 60 * 60 * 24
	} else if strings.HasSuffix(str, "w") || strings.HasSuffix(str, "W") {
		mult = 60 * 60 * 24 * 7
	}

	var value uint64
	if mult > 1 {
		if value, err = strconv.ParseUint(str[:len(str)-1], 10, 64); err != nil {
			return 0, err
		}
	} else {
		if value, err = strconv.ParseUint(str, 10, 64); err != nil {
			return 0, err
		}
	}

	var upperBound = math.MaxUint64 / mult
	if value > upperBound {
		return 0, fmt.Errorf("number of seconds is too big: '%v'", str)
	}

	return value * mult, nil
}

func stringToPortNumber(str string) (port uint32, err error) {
	var ui64 uint64
	if ui64, err = strconv.ParseUint(str, 10, 32); err != nil {
		return 0, fmt.Errorf("invalid port number '%v'", str)
	}
	if ui64 > 65535 {
		return 0, fmt.Errorf("invalid port number '%v'", str)
	}
	return uint32(ui64), nil
}

func stringToSeverityLevel(str string) (level uint, err error) {
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
		return 0, fmt.Errorf("invalid severity level %v", str)
	}
}

func severityLevelToString(t int) string {
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
		}
		return filepath.Dir(SiodbInstanceConfigurationPath) + `/` + fileName, nil
	}
	return fileName, nil
}

func minUint32(a uint32, b uint32) uint32 {
	if a < b {
		return a
	}
	return b
}
