package main

import (
	"encoding/base64"
	"fmt"
	"strings"

	"github.com/gin-gonic/gin"
)

func loadAuthenticationData(c *gin.Context) (UserName string, Token string, err error) {

	var userPassBase64 string

	// Get Token
	if len(c.Request.Header.Get("Authorization")) > 0 {
		if c.Request.Header.Get("Authorization")[0:5] != "Basic" {
			return UserName, Token, fmt.Errorf("Invalid authorization scheme, expecting basic authorization scheme")
		}
		userPassBase64 = c.Request.Header.Get("Authorization")[6:]
	} else {
		return UserName, Token, fmt.Errorf("Invalid authorization information provided")
	}

	userPass, err := base64.StdEncoding.DecodeString(userPassBase64)
	if err != nil {
		return UserName, Token, fmt.Errorf("Invalid authorization information provided")
	}

	if strings.Count(string(userPass), ":") != 1 {
		return UserName, Token, fmt.Errorf("Invalid authorization information provided")
	}
	splittedUserPass := strings.Split(string(userPass), ":")
	UserName = splittedUserPass[0]
	Token = splittedUserPass[1]

	return UserName, Token, nil
}
