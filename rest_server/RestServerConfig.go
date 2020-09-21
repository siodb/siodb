package main

import (
	"fmt"
)

var (
	HTTPChunkSizeMinSize uint64 = 1024
	HTTPChunkSizeMaxSize uint64 = 1 * 1024 * 1024
)

type RestServerConfig struct {
	Ipv4HTTPPort   uint32
	Ipv4HTTPSPort  uint32
	Ipv6HTTPPort   uint32
	Ipv6HTTPSPort  uint32
	TLSCertificate string
	TLSPrivateKey  string
	HTTPChunkSize  uint64
}

func (rsc *RestServerConfig) ParseAndValidateConfiguration(siodbConfigFile *SiodbConfigFile) (err error) {

	var value string

	// REST Ports
	if value, err = siodbConfigFile.GetParameterValue("rest_server.ipv4_http_port"); err != nil {
		return fmt.Errorf("error for parameter 'rest_server.ipv4_http_port': %v", err)
	}
	if rsc.Ipv4HTTPPort, err = StringToPortNumber(value); err != nil {
		return fmt.Errorf("error for parameter 'rest_server.ipv4_http_port': %v", err)
	}

	if value, err = siodbConfigFile.GetParameterValue("rest_server.ipv4_https_port"); err != nil {
		return fmt.Errorf("error for parameter 'rest_server.ipv4_https_port': %v", err)
	}
	if rsc.Ipv4HTTPSPort, err = StringToPortNumber(value); err != nil {
		return fmt.Errorf("error for parameter 'rest_server.ipv4_https_port': %v", err)
	}

	if value, err = siodbConfigFile.GetParameterValue("rest_server.ipv6_http_port"); err != nil {
		return fmt.Errorf("error for parameter 'rest_server.ipv6_http_port': %v", err)
	}
	if rsc.Ipv6HTTPPort, err = StringToPortNumber(value); err != nil {
		return fmt.Errorf("error for parameter 'rest_server.ipv6_http_port': %v", err)
	}

	if value, err = siodbConfigFile.GetParameterValue("rest_server.ipv6_https_port"); err != nil {
		return fmt.Errorf("error for parameter 'rest_server.ipv6_https_port': %v", err)
	}
	if rsc.Ipv6HTTPSPort, err = StringToPortNumber(value); err != nil {
		return fmt.Errorf("error for parameter 'rest_server.ipv6_https_port': %v", err)
	}

	// TLS
	if value, err = siodbConfigFile.GetParameterValue("rest_server.tls_certificate_chain"); err != nil {
		if value, err = siodbConfigFile.GetParameterValue("rest_server.tls_certificate"); err != nil {
			return fmt.Errorf("unable to find parameter for tls certificate '%v'", err)
		} else {
			if rsc.TLSCertificate, err = verifyPath(siodbConfigFile.path, value); err != nil {
				return fmt.Errorf("error for parameter 'rest_server.tls_certificate': %v", err)
			}
		}
	} else {
		if rsc.TLSCertificate, err = verifyPath(siodbConfigFile.path, value); err != nil {
			return fmt.Errorf("error for parameter 'rest_server.tls_certificate': %v", err)
		}
	}

	if value, err = siodbConfigFile.GetParameterValue("rest_server.tls_private_key"); err != nil {
		return fmt.Errorf("error for parameter 'rest_server.tls_private_key': %v", err)
	}
	if rsc.TLSPrivateKey, err = verifyPath(siodbConfigFile.path, value); err != nil {
		return fmt.Errorf("error for parameter 'rest_server.tls_private_key': %v", err)
	}

	// Chunk size
	if value, err = siodbConfigFile.GetParameterValue("rest_server.chunk_size"); err != nil {
		return err
	}
	if rsc.HTTPChunkSize, err = StringToByteSize(value); err != nil {
		return fmt.Errorf("error for parameter 'rest_server.chunk_size': %v", err)
	}
	if rsc.HTTPChunkSize < HTTPChunkSizeMinSize || rsc.HTTPChunkSize > HTTPChunkSizeMaxSize {
		return fmt.Errorf("parameter 'rest_server.chunk_size' (%v) is out of range (%v-%v)",
			value, HTTPChunkSizeMinSize, HTTPChunkSizeMaxSize)
	}

	return nil
}
