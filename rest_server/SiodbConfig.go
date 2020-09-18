package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"strings"
)

type Config map[string]string

const (
	CONSOLE = 1
	FILE    = 2
)

// LogOptions is an implementation of the original LogOptions from Siodb
// https://github.com/siodb/siodb/blob/master/common/lib/siodb/common/options/LogOptions.h
type LogOptions struct {
	channelName              string
	channelType              int
	destination              string
	maxLogFileSize           uint64
	maxFiles                 uint64
	logFileExpirationTimeout uint64
	severityLevel            uint64
}

// func parseConfiguration(SiodbInstanceConfigurationPath string, config Config) (err error) {

// 	// Parse and validate parameters
// 	if len(config["log.file.destination"]) == 0 {
// 		return fmt.Errorf("missing parameter 'log.file.destination'")
// 	}
// 	rs.logFileDestination = config["log.file.destination"]
// 	if len(config["log.file.severity"]) == 0 {
// 		return fmt.Errorf("missing parameter 'log.file.severity'")
// 	}
// 	rs.logFileSeverity = config["log.file.severity"]
// 	if len(config["log.console.destination"]) == 0 {
// 		return fmt.Errorf("missing parameter 'log.console.destination'")
// 	}
// 	rs.logConsoleDestination = config["log.console.destination"]
// 	if len(config["log.console.severity"]) == 0 {
// 		return fmt.Errorf("missing parameter 'log.console.severity'")
// 	}
// 	rs.logConsoleSeverity = config["log.console.severity"]

// 	if rs.Ipv4HTTPPort, err = strconv.ParseUint(config["rest_server.ipv4_http_port"], 10, 64); err != nil {
// 		return err
// 	}
// 	if rs.Ipv4HTTPSPort, err = strconv.ParseUint(config["rest_server.ipv4_https_port"], 10, 64); err != nil {
// 		return err
// 	}
// 	if rs.Ipv6HTTPPort, err = strconv.ParseUint(config["rest_server.ipv6_http_port"], 10, 64); err != nil {
// 		return err
// 	}
// 	if rs.Ipv6HTTPSPort, err = strconv.ParseUint(config["rest_server.ipv6_https_port"], 10, 64); err != nil {
// 		return err
// 	}
// 	if len(config["rest_server.tls_certificate_chain"]) > 0 {
// 		if _, err := os.Stat(config["rest_server.tls_certificate_chain"]); os.IsNotExist(err) {
// 			if _, err := os.Stat(SiodbInstanceConfigurationPath + `/` + config["rest_server.tls_certificate_chain"]); os.IsNotExist(err) {
// 				return err
// 			} else {
// 				rs.TLSCertificate = SiodbInstanceConfigurationPath + `/` + config["rest_server.tls_certificate_chain"]
// 			}

// 		} else {
// 			rs.TLSCertificate = config["rest_server.tls_certificate_chain"]
// 		}
// 	} else {
// 		if _, err := os.Stat(config["rest_server.tls_certificate"]); os.IsNotExist(err) {
// 			if _, err := os.Stat(SiodbInstanceConfigurationPath + `/` + config["rest_server.tls_certificate"]); os.IsNotExist(err) {
// 				return err
// 			} else {
// 				rs.TLSCertificate = SiodbInstanceConfigurationPath + `/` + config["rest_server.tls_certificate"]
// 			}
// 		} else {
// 			rs.TLSCertificate = config["rest_server.tls_certificate"]
// 		}
// 	}
// 	if _, err := os.Stat(config["rest_server.tls_private_key"]); os.IsNotExist(err) {
// 		if _, err := os.Stat(SiodbInstanceConfigurationPath + `/` + config["rest_server.tls_private_key"]); os.IsNotExist(err) {
// 			return err
// 		} else {
// 			rs.TLSPrivateKey = SiodbInstanceConfigurationPath + `/` + config["rest_server.tls_private_key"]
// 		}
// 	} else {
// 		rs.TLSPrivateKey = config["rest_server.tls_private_key"]
// 	}

// 	// Parse rest_server.chunk_size
// 	var Unit string = config["rest_server.chunk_size"][len(config["rest_server.chunk_size"])-1:]
// 	var IsLetter = regexp.MustCompile(`^[a-zA-Z]+$`).MatchString

// 	if IsLetter(Unit) {
// 		if rs.HTTPChunkSize, err = strconv.ParseUint(config["rest_server.chunk_size"][:len(config["rest_server.chunk_size"])-1], 10, 64); err != nil {
// 			return err
// 		}
// 		switch Unit {
// 		case "k", "K":
// 			rs.HTTPChunkSize = rs.HTTPChunkSize * 1024
// 		case "m", "M":
// 			rs.HTTPChunkSize = rs.HTTPChunkSize * 1024 * 1024
// 		default:
// 			return fmt.Errorf("unknown unit '%v' for parameter 'rest_server.chunk_size'", Unit)
// 		}
// 	} else {
// 		if rs.HTTPChunkSize, err = strconv.ParseUint(config["rest_server.chunk_size"], 10, 64); err != nil {
// 			return err
// 		}
// 	}

// 	return nil
// }

func parseConfig(filename string, parameter string) (string, error) {

	if len(filename) == 0 {
		return "", nil
	}
	file, err := os.Open(filename)
	if err != nil {
		return "", err
	}
	defer file.Close()

	reader := bufio.NewReader(file)

	value := ""
	for {
		line, err := reader.ReadString('\n')

		if !strings.HasPrefix(line, "#") && len(line) > 0 {
			if strings.HasPrefix(strings.TrimSpace(line), parameter) {
				if equal := strings.Index(line, "="); equal >= 0 {
					if key := strings.TrimSpace(line[:equal]); len(key) > 0 {
						if len(line) > equal {
							value = strings.TrimSpace(line[equal+1:])
						}
					}
				}
			}
		}
		if err == io.EOF {
			break
		}
		if err != nil {
			return "", err
		}
	}
	return value, nil
}

func main() {

	v, e := parseConfig("/etc/siodb/instances/siodb000/config", "rest_server.tls_private_key")
	fmt.Printf("%v,%v\n", v, e)

}
