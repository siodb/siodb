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

func (scf SiodbConfigFile) ParseLogChannelName() (channel []string, err error) {

	var value string
	if value, err = scf.GetParameterValue("log_channels"); err != nil {
		return channel, err
	}
	for _, c := range strings.Split(value, ",") {
		channel = append(channel, strings.ToLower(strings.TrimSpace(c)))
	}
	return channel, nil
}

func (scf *SiodbConfigFile) GetParameterValue(parameter string) (string, error) {

	for _, param := range scf.parameter {
		if param.name == parameter {
			return param.value, nil
		}
	}

	return "", fmt.Errorf("the parameter '%v' doesn't exist in the parameter file", parameter)

}

func (scf *SiodbConfigFile) ParseParameters() (err error) {

	if len(scf.path) == 0 {
		return nil
	}
	file, err := os.Open(scf.path)
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
						scf.parameter = append(scf.parameter, *parameter)
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
	c, e := siodbConfigFile.ParseLogChannelName()
	fmt.Printf("Channel: %v | error: %v\n", c, e)

}
