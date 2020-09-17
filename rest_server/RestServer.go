package main

// specs from https://github.com/siodb/siodb/blob/master/docs/dev/designs/RestServer.md
// TODO: Implement "basic" authentication as describe in RestServer spec
// TODO: Parse parameters into a structures
// TODO: Protect IOMgrbuff with a limit to which it will start to stream
// TODO: Finish the POST and DELETE implementation
// TODO: log format
// TODO: Put chunked WritePayload in function
// TODO: Put read chunked payload from IOMgr in function
// TODO: Once cxxman code step 5 in https://github.com/siodb/siodb/blob/master/docs/dev/designs/RestServer.md#post-patch-requests

import (
	"bufio"
	"flag"
	"fmt"
	"io"
	"log"
	"os"
	"os/signal"
	"regexp"
	"strconv"
	"strings"
	"syscall"
)

type RestServer struct {
	Ipv4HTTPPort          uint64
	Ipv4HTTPSPort         uint64
	Ipv6HTTPPort          uint64
	Ipv6HTTPSPort         uint64
	TLSCertificate        string
	TLSPrivateKey         string
	HTTPChunkSize         uint64
	logFileDestination    string
	logFileSeverity       string
	logConsoleDestination string
	logConsoleSeverity    string
}

type Config map[string]string

var (
	SiodbInstanceConfigurationRootPath string = "/etc/siodb/instances"
	restServer                         RestServer
	IOMgrCPool                         *IOMgrConnPool
	siodbLoggerPool                    SiodbLoggerPool
)

var (
	DATABASEENGINERESPONSE    uint64 = 4
	DATABASEENGINERESTREQUEST uint64 = 13
)

func main() {

	// Parse Args
	SiodbInstanceName := flag.String("instance", "siodb", "Instance name")
	flag.Parse()

	// Signals
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGTERM)

	// Variables
	SiodbInstanceConfigurationPath := SiodbInstanceConfigurationRootPath + "/" + *SiodbInstanceName
	var err error
	var config Config

	// Parse Siodb instance parameters file
	if config, err = parseInitFile(SiodbInstanceConfigurationPath + "/config"); err != nil {
		log.Fatalln(err)
	}

	// Validate and load parameters
	if err = restServer.parseConfiguration(SiodbInstanceConfigurationPath, config); err != nil {
		log.Fatalln(err)
	}

	// Logging setup
	siodbLoggerPool.AddSiodbLogger(restServer.logConsoleSeverity, restServer.logConsoleDestination)
	siodbLoggerPool.AddSiodbLogger(restServer.logFileSeverity, restServer.logFileDestination)

	siodbLoggerPool.Output(INFO, "Starting with config: %v", restServer)

	// Gin Log at global level
	siodbLoggerPool.ConfigGinLogger()

	// Connection pool to IOMgr
	IOMgrCPool, err = CreateIOMgrConnPool(4, 8, config)
	if err != nil {
		siodbLoggerPool.Output(FATAL, "Cannot create connection pool: %v", err)
	}

	// Start routers
	if restServer.Ipv4HTTPPort != 0 {
		go func() {
			var restWorker RestWorker
			if err := restWorker.CreateRouter(restServer.Ipv4HTTPPort); err != nil {
				siodbLoggerPool.Output(FATAL, "Cannot create route: %v", err)
			}
			if err := restWorker.StartHTTPRouter(); err != nil {
				siodbLoggerPool.Output(FATAL, "Cannot start router: %v", err)
			}
		}()

	}
	if restServer.Ipv4HTTPSPort != 0 {
		go func() {
			var restWorker RestWorker
			if err := restWorker.CreateRouter(restServer.Ipv4HTTPSPort); err != nil {
				siodbLoggerPool.Output(FATAL, "Cannot create route: %v", err)
			}
			if err := restWorker.StartHTTPSRouter(restServer.TLSCertificate, restServer.TLSPrivateKey); err != nil {
				siodbLoggerPool.Output(FATAL, "Cannot start router: %v", err)
			}
		}()
	}
	if restServer.Ipv6HTTPPort != 0 {
		go func() {
			var restWorker RestWorker
			if err := restWorker.CreateRouter(restServer.Ipv6HTTPPort); err != nil {
				siodbLoggerPool.Output(FATAL, "Cannot create route: %v", err)
			}
			if err := restWorker.StartHTTPRouter(); err != nil {
				siodbLoggerPool.Output(FATAL, "Cannot start router: %v", err)
			}
		}()
	}
	if restServer.Ipv6HTTPSPort != 0 {
		go func() {
			var restWorker RestWorker
			if err := restWorker.CreateRouter(restServer.Ipv6HTTPSPort); err != nil {
				siodbLoggerPool.Output(FATAL, "Cannot create route: %v", err)
			}
			if err := restWorker.StartHTTPSRouter(restServer.TLSCertificate, restServer.TLSPrivateKey); err != nil {
				siodbLoggerPool.Output(FATAL, "Cannot start router: %v", err)
			}
		}()
	}

	sig := <-sigs
	siodbLoggerPool.Output(INFO, "Signal received: %v", sig)

}

func (rs *RestServer) parseConfiguration(SiodbInstanceConfigurationPath string, config Config) (err error) {

	// Parse and validate parameters
	if len(config["log.file.destination"]) == 0 {
		return fmt.Errorf("missing parameter 'log.file.destination'")
	}
	rs.logFileDestination = config["log.file.destination"]
	if len(config["log.file.severity"]) == 0 {
		return fmt.Errorf("missing parameter 'log.file.severity'")
	}
	rs.logFileSeverity = config["log.file.severity"]
	if len(config["log.console.destination"]) == 0 {
		return fmt.Errorf("missing parameter 'log.console.destination'")
	}
	rs.logConsoleDestination = config["log.console.destination"]
	if len(config["log.console.severity"]) == 0 {
		return fmt.Errorf("missing parameter 'log.console.severity'")
	}
	rs.logConsoleSeverity = config["log.console.severity"]

	if rs.Ipv4HTTPPort, err = strconv.ParseUint(config["rest_server.ipv4_http_port"], 10, 64); err != nil {
		return err
	}
	if rs.Ipv4HTTPSPort, err = strconv.ParseUint(config["rest_server.ipv4_https_port"], 10, 64); err != nil {
		return err
	}
	if rs.Ipv6HTTPPort, err = strconv.ParseUint(config["rest_server.ipv6_http_port"], 10, 64); err != nil {
		return err
	}
	if rs.Ipv6HTTPSPort, err = strconv.ParseUint(config["rest_server.ipv6_https_port"], 10, 64); err != nil {
		return err
	}
	if len(config["rest_server.tls_certificate_chain"]) > 0 {
		if _, err := os.Stat(config["rest_server.tls_certificate_chain"]); os.IsNotExist(err) {
			if _, err := os.Stat(SiodbInstanceConfigurationPath + `/` + config["rest_server.tls_certificate_chain"]); os.IsNotExist(err) {
				return err
			} else {
				rs.TLSCertificate = SiodbInstanceConfigurationPath + `/` + config["rest_server.tls_certificate_chain"]
			}

		} else {
			rs.TLSCertificate = config["rest_server.tls_certificate_chain"]
		}
	} else {
		if _, err := os.Stat(config["rest_server.tls_certificate"]); os.IsNotExist(err) {
			if _, err := os.Stat(SiodbInstanceConfigurationPath + `/` + config["rest_server.tls_certificate"]); os.IsNotExist(err) {
				return err
			} else {
				rs.TLSCertificate = SiodbInstanceConfigurationPath + `/` + config["rest_server.tls_certificate"]
			}
		} else {
			rs.TLSCertificate = config["rest_server.tls_certificate"]
		}
	}
	if _, err := os.Stat(config["rest_server.tls_private_key"]); os.IsNotExist(err) {
		if _, err := os.Stat(SiodbInstanceConfigurationPath + `/` + config["rest_server.tls_private_key"]); os.IsNotExist(err) {
			return err
		} else {
			rs.TLSPrivateKey = SiodbInstanceConfigurationPath + `/` + config["rest_server.tls_private_key"]
		}
	} else {
		rs.TLSPrivateKey = config["rest_server.tls_private_key"]
	}

	// Parse rest_server.chunk_size
	var Unit string = config["rest_server.chunk_size"][len(config["rest_server.chunk_size"])-1:]
	var IsLetter = regexp.MustCompile(`^[a-zA-Z]+$`).MatchString

	if IsLetter(Unit) {
		if rs.HTTPChunkSize, err = strconv.ParseUint(config["rest_server.chunk_size"][:len(config["rest_server.chunk_size"])-1], 10, 64); err != nil {
			return err
		}
		switch Unit {
		case "k", "K":
			rs.HTTPChunkSize = rs.HTTPChunkSize * 1024
		case "m", "M":
			rs.HTTPChunkSize = rs.HTTPChunkSize * 1024 * 1024
		default:
			return fmt.Errorf("unknown unit '%v' for parameter 'rest_server.chunk_size'", Unit)
		}
	} else {
		if rs.HTTPChunkSize, err = strconv.ParseUint(config["rest_server.chunk_size"], 10, 64); err != nil {
			return err
		}
	}

	return nil
}

func parseInitFile(filename string) (Config, error) {

	// Source: https://www.socketloop.com/tutorials/golang-read-data-from-config-file-and-assign-to-variables

	config := Config{}

	if len(filename) == 0 {
		return config, nil
	}
	file, err := os.Open(filename)
	if err != nil {
		return nil, err
	}
	defer file.Close()

	reader := bufio.NewReader(file)

	for {
		line, err := reader.ReadString('\n')

		if !strings.HasPrefix(line, "#") && len(line) > 0 {
			if equal := strings.Index(line, "="); equal >= 0 {
				if key := strings.TrimSpace(line[:equal]); len(key) > 0 {
					value := ""
					if len(line) > equal {
						value = strings.TrimSpace(line[equal+1:])
					}
					// assign the config map
					config[key] = value
				}
			}
		}
		if err == io.EOF {
			break
		}
		if err != nil {
			return nil, err
		}
	}
	return config, nil
}
