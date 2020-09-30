// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

package main

import (
	"fmt"
)

var (
	HTTPChunkSizeMinSize        uint32 = 1 * 1024
	HTTPChunkSizeMaxSize        uint32 = 1 * 1024 * 1024
	RequestPayloadMinBufferSize uint32 = 1 * 1024
	RequestPayloadMaxBufferSize uint32 = 10 * 1024 * 1024
)

type RestServerConfig struct {
	Ipv4HTTPPort             uint32
	Ipv4HTTPSPort            uint32
	Ipv6HTTPPort             uint32
	Ipv6HTTPSPort            uint32
	TLSCertificate           string
	TLSPrivateKey            string
	HTTPChunkSize            uint32
	RequestPayloadBufferSize uint32
}

func (restServerConfig *RestServerConfig) ParseAndValidateConfiguration(siodbConfigFile *SiodbConfigFile) (err error) {

	var value string

	// REST Ports
	if value, err = siodbConfigFile.GetParameterValue("rest_server.ipv4_http_port"); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.ipv4_http_port': %v", err)
	}
	if restServerConfig.Ipv4HTTPPort, err = StringToPortNumber(value); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.ipv4_http_port': %v", err)
	}

	if value, err = siodbConfigFile.GetParameterValue("rest_server.ipv4_https_port"); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.ipv4_https_port': %v", err)
	}
	if restServerConfig.Ipv4HTTPSPort, err = StringToPortNumber(value); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.ipv4_https_port': %v", err)
	}

	if value, err = siodbConfigFile.GetParameterValue("rest_server.ipv6_http_port"); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.ipv6_http_port': %v", err)
	}
	if restServerConfig.Ipv6HTTPPort, err = StringToPortNumber(value); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.ipv6_http_port': %v", err)
	}

	if value, err = siodbConfigFile.GetParameterValue("rest_server.ipv6_https_port"); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.ipv6_https_port': %v", err)
	}
	if restServerConfig.Ipv6HTTPSPort, err = StringToPortNumber(value); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.ipv6_https_port': %v", err)
	}

	// TLS
	if value, err = siodbConfigFile.GetParameterValue("rest_server.tls_certificate_chain"); err != nil {
		if value, err = siodbConfigFile.GetParameterValue("rest_server.tls_certificate"); err != nil {
			return fmt.Errorf("unable to find parameter for tls certificate '%v'", err)
		} else {
			if restServerConfig.TLSCertificate, err = verifyPath(siodbConfigFile.path, value); err != nil {
				return fmt.Errorf("Invalid parameter 'rest_server.tls_certificate': %v", err)
			}
		}
	} else {
		if restServerConfig.TLSCertificate, err = verifyPath(siodbConfigFile.path, value); err != nil {
			return fmt.Errorf("Invalid parameter 'rest_server.tls_certificate': %v", err)
		}
	}

	if value, err = siodbConfigFile.GetParameterValue("rest_server.tls_private_key"); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.tls_private_key': %v", err)
	}
	if restServerConfig.TLSPrivateKey, err = verifyPath(siodbConfigFile.path, value); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.tls_private_key': %v", err)
	}

	// HTTP chunk_size
	if value, err = siodbConfigFile.GetParameterValue("rest_server.chunk_size"); err != nil {
		return err
	}
	if restServerConfig.HTTPChunkSize, err = StringToByteSize(value); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.chunk_size': %v", err)
	}
	if restServerConfig.HTTPChunkSize < HTTPChunkSizeMinSize || restServerConfig.HTTPChunkSize > HTTPChunkSizeMaxSize {
		return fmt.Errorf("Invalid parameter: 'rest_server.chunk_size' (%v) is out of range (%v-%v)",
			value, HTTPChunkSizeMinSize, HTTPChunkSizeMaxSize)
	}

	// request_payload_buffer_size
	if value, err = siodbConfigFile.GetParameterValue("rest_server.request_payload_buffer_size"); err != nil {
		return err
	}
	if restServerConfig.RequestPayloadBufferSize, err = StringToByteSize(value); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.request_payload_buffer_size': %v", err)
	}
	if restServerConfig.RequestPayloadBufferSize < RequestPayloadMinBufferSize || restServerConfig.RequestPayloadBufferSize > RequestPayloadMaxBufferSize {
		return fmt.Errorf("Invalid parameter: 'rest_server.request_payload_buffer_size' (%v) is out of range (%v-%v)",
			value, RequestPayloadMinBufferSize, RequestPayloadMaxBufferSize)
	}

	return nil
}
