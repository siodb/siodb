// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"strings"
	"testing"
)

type siodbConfigFile struct {
	path      string
	parameter []configurationParameter
}

type configurationParameter struct {
	name  string
	value string
}

func (configFile *siodbConfigFile) ParseLogChannelsName() (channel []string, err error) {
	var value string
	if value, err = configFile.GetParameterValue("log_channels"); err != nil {
		return channel, err
	}
	for _, c := range strings.Split(value, ",") {
		channel = append(channel, strings.ToLower(strings.TrimSpace(c)))
	}
	return channel, nil
}

func (configFile *siodbConfigFile) GetParameterValue(parameter string) (string, error) {
	for _, param := range configFile.parameter {
		if param.name == parameter {
			return param.value, nil
		}
	}

	return "", fmt.Errorf("configurationParameter '%v' doesn't exist in the parameter file", parameter)
}

func (configFile *siodbConfigFile) ParseParameters() (err error) {
	if len(configFile.path) == 0 {
		return nil
	}
	file, err := os.Open(configFile.path)
	if err != nil {
		return err
	}
	defer file.Close()

	reader := bufio.NewReader(file)

	for {
		line, err := reader.ReadString('\n')
		if err == io.EOF {
			break
		}
		if err != nil {
			return err
		}

		parameter := &configurationParameter{}

		if !strings.HasPrefix(strings.TrimSpace(line), "#") && len(line) > 0 {
			if equal := strings.Index(line, "="); equal >= 0 {
				if key := strings.TrimSpace(line[:equal]); len(key) > 0 {
					if len(line) > equal {
						parameter.name = key
						parameter.value = strings.TrimSpace(line[equal+1:])
						configFile.parameter = append(configFile.parameter, *parameter)
					}
				}
			}
		}
	}

	return nil
}

func testConfig(t *testing.T) {
	siodbConfigFile := &siodbConfigFile{}
	siodbConfigFile.path = "/etc/siodb/instances/siodb000/config"
	err := siodbConfigFile.ParseParameters()
	fmt.Printf("siodbConfigFile: %v | error: %v.\n", siodbConfigFile, err)
	v, e := siodbConfigFile.GetParameterValue("rest_server.tls_private_key")
	fmt.Printf("rest_server.tls_private_key: %v| error: %v\n", v, e)
	c, e := siodbConfigFile.ParseLogChannelsName()
	fmt.Printf("Channel: %v | error: %v\n", c, e)
}
