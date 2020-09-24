package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"strings"
	"testing"
)

type SiodbConfigFile struct {
	path      string
	parameter []Parameter
}

type Parameter struct {
	name  string
	value string
}

func (siodbConfigFile SiodbConfigFile) ParseLogChannelsName() (channel []string, err error) {

	var value string
	if value, err = siodbConfigFile.GetParameterValue("log_channels"); err != nil {
		return channel, err
	}
	for _, c := range strings.Split(value, ",") {
		channel = append(channel, strings.ToLower(strings.TrimSpace(c)))
	}
	return channel, nil
}

func (siodbConfigFile *SiodbConfigFile) GetParameterValue(parameter string) (string, error) {

	for _, param := range siodbConfigFile.parameter {
		if param.name == parameter {
			return param.value, nil
		}
	}

	return "", fmt.Errorf("parameter '%v' doesn't exist in the parameter file", parameter)
}

func (siodbConfigFile *SiodbConfigFile) ParseParameters() (err error) {

	if len(siodbConfigFile.path) == 0 {
		return nil
	}
	file, err := os.Open(siodbConfigFile.path)
	if err != nil {
		return err
	}
	defer file.Close()

	reader := bufio.NewReader(file)

	for {
		line, err := reader.ReadString('\n')

		parameter := &Parameter{}

		if !strings.HasPrefix(strings.TrimSpace(line), "#") && len(line) > 0 {
			if equal := strings.Index(line, "="); equal >= 0 {
				if key := strings.TrimSpace(line[:equal]); len(key) > 0 {
					if len(line) > equal {
						parameter.name = key
						parameter.value = strings.TrimSpace(line[equal+1:])
						siodbConfigFile.parameter = append(siodbConfigFile.parameter, *parameter)
					}
				}
			}
		}
		if err == io.EOF {
			break
		}
		if err != nil {
			return err
		}
	}

	return nil
}

func TestConfig(t *testing.T) {

	siodbConfigFile := &SiodbConfigFile{}
	siodbConfigFile.path = "/etc/siodb/instances/siodb000/config"
	err := siodbConfigFile.ParseParameters()
	fmt.Printf("siodbConfigFile: %v | error: %v.\n", siodbConfigFile, err)
	v, e := siodbConfigFile.GetParameterValue("rest_server.tls_private_key")
	fmt.Printf("rest_server.tls_private_key: %v| error: %v\n", v, e)
	c, e := siodbConfigFile.ParseLogChannelsName()
	fmt.Printf("Channel: %v | error: %v\n", c, e)
}
