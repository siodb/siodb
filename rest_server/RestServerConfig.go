// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

package main

import (
	"fmt"
)

var (
	httpChunkSizeMinSize        uint32 = 1 * 1024
	httpChunkSizeMaxSize        uint32 = 1 * 1024 * 1024
	requestPayloadMinBufferSize uint32 = 1 * 1024
	requestPayloadMaxBufferSize uint32 = 10 * 1024 * 1024
)

type restServerConfig struct {
	ipv4HTTPPort             uint32
	ipv4HTTPSPort            uint32
	ipv6HTTPPort             uint32
	ipv6HTTPSPort            uint32
	tlsCertificate           string
	tlsPrivateKey            string
	httpChunkSize            uint32
	requestPayloadBufferSize uint32
}

func (restServerConfig *restServerConfig) ParseAndValidateConfiguration(siodbConfigFile *siodbConfigFile) (err error) {
	var value string

	// REST Ports
	if value, err = siodbConfigFile.GetParameterValue("rest_server.ipv4_http_port"); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.ipv4_http_port': %v", err)
	}
	if restServerConfig.ipv4HTTPPort, err = stringToPortNumber(value); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.ipv4_http_port': %v", err)
	}

	if value, err = siodbConfigFile.GetParameterValue("rest_server.ipv4_https_port"); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.ipv4_https_port': %v", err)
	}
	if restServerConfig.ipv4HTTPSPort, err = stringToPortNumber(value); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.ipv4_https_port': %v", err)
	}

	if value, err = siodbConfigFile.GetParameterValue("rest_server.ipv6_http_port"); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.ipv6_http_port': %v", err)
	}
	if restServerConfig.ipv6HTTPPort, err = stringToPortNumber(value); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.ipv6_http_port': %v", err)
	}

	if value, err = siodbConfigFile.GetParameterValue("rest_server.ipv6_https_port"); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.ipv6_https_port': %v", err)
	}
	if restServerConfig.ipv6HTTPSPort, err = stringToPortNumber(value); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.ipv6_https_port': %v", err)
	}

	// TLS
	if restServerConfig.ipv4HTTPSPort != 0 || restServerConfig.ipv6HTTPSPort != 0 {
		if value, err = siodbConfigFile.GetParameterValue("rest_server.tls_certificate_chain"); err != nil {
			if value, err = siodbConfigFile.GetParameterValue("rest_server.tls_certificate"); err != nil {
				return fmt.Errorf("Can't find parameter for TLS certificate: there is no valid parameter " +
					"rest_server.tls_certificate_chain or rest_server.tls_certificate")
			}
			if restServerConfig.tlsCertificate, err = verifyPath(siodbConfigFile.path, value); err != nil {
				return fmt.Errorf("Invalid parameter 'rest_server.tls_certificate': %v", err)
			}
		} else {
			if restServerConfig.tlsCertificate, err = verifyPath(siodbConfigFile.path, value); err != nil {
				return fmt.Errorf("Invalid parameter 'rest_server.tls_certificate_chain': %v", err)
			}
		}

		if value, err = siodbConfigFile.GetParameterValue("rest_server.tls_private_key"); err != nil {
			return fmt.Errorf("Invalid parameter 'rest_server.tls_private_key': %v", err)
		}
		if restServerConfig.tlsPrivateKey, err = verifyPath(siodbConfigFile.path, value); err != nil {
			return fmt.Errorf("Invalid parameter 'rest_server.tls_private_key': %v", err)
		}
	}

	// HTTP chunk_size
	if value, err = siodbConfigFile.GetParameterValue("rest_server.chunk_size"); err != nil {
		return err
	}
	if restServerConfig.httpChunkSize, err = stringToByteSize(value); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.chunk_size': %v", err)
	}
	if restServerConfig.httpChunkSize < httpChunkSizeMinSize || restServerConfig.httpChunkSize > httpChunkSizeMaxSize {
		return fmt.Errorf("Invalid parameter: 'rest_server.chunk_size' (%v) is out of range (%v-%v)",
			value, httpChunkSizeMinSize, httpChunkSizeMaxSize)
	}

	// request_payload_buffer_size
	if value, err = siodbConfigFile.GetParameterValue("rest_server.request_payload_buffer_size"); err != nil {
		return err
	}
	if restServerConfig.requestPayloadBufferSize, err = stringToByteSize(value); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.request_payload_buffer_size': %v", err)
	}
	if restServerConfig.requestPayloadBufferSize < requestPayloadMinBufferSize || restServerConfig.requestPayloadBufferSize > requestPayloadMaxBufferSize {
		return fmt.Errorf("Invalid parameter: 'rest_server.request_payload_buffer_size' (%v) is out of range (%v-%v)",
			value, requestPayloadMinBufferSize, requestPayloadMaxBufferSize)
	}

	return nil
}
