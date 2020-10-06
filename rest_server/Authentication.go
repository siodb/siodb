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
	var userPassBase64 string

	// Get token
	if len(c.Request.Header.Get("Authorization")) > 0 {
		if c.Request.Header.Get("Authorization")[0:5] != "Basic" {
			return userName, token, fmt.Errorf("Invalid authorization scheme, expecting basic authorization scheme")
		}
		userPassBase64 = c.Request.Header.Get("Authorization")[6:]
	} else {
		return userName, token, fmt.Errorf("Invalid authorization information provided")
	}

	userPass, err := base64.StdEncoding.DecodeString(userPassBase64)
	if err != nil {
		return userName, token, fmt.Errorf("Invalid authorization information provided")
	}

	if strings.Count(string(userPass), ":") != 1 {
		return userName, token, fmt.Errorf("Invalid authorization information provided")
	}
	splittedUserPass := strings.Split(string(userPass), ":")
	userName = splittedUserPass[0]
	token = splittedUserPass[1]

	return userName, token, nil
}
