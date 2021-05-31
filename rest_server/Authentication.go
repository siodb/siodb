// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

package main

import (
	"encoding/base64"
	"fmt"
	"strings"

	"github.com/gin-gonic/gin"
)

func loadAuthenticationData(c *gin.Context) (userName string, token string, err error) {
	var userPassStr string
	var header = c.Request.Header.Get("Authorization")
	var headerFields = strings.Fields(header)

	if len(headerFields) != 2 {
		return userName, token, fmt.Errorf("invalid authorization information provided")
	}
	if headerFields[0] != "Basic" {
		return userName, token, fmt.Errorf("invalid authorization scheme, expecting 'Basic' authorization scheme")
	}

	userPassBytes, err := base64.StdEncoding.DecodeString(headerFields[1])
	if err != nil {
		return userName, token, fmt.Errorf("invalid authorization information provided")
	}

	// Important: Remove trailing newline from token, otherwise authentication fails
	userPassStr = strings.TrimSuffix(string(userPassBytes), "\n")

	if strings.Count(userPassStr, ":") != 1 {
		return userName, token, fmt.Errorf("invalid authorization information provided")
	}

	splittedUserPass := strings.Split(userPassStr, ":")
	userName = splittedUserPass[0]
	token = splittedUserPass[1]

	return userName, token, nil
}
