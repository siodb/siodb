package main

import (
	"fmt"

	"github.com/gin-gonic/gin"
)

func loadAuthenticationData(c *gin.Context) (UserName string, Token string, err error) {

	// Get Username
	if len(c.Request.Header.Get("Username")) > 0 {
		UserName = c.Request.Header.Get("Username")
	} else {
		UserName = "root"
	}

	// Get Token
	if len(c.Request.Header.Get("Authorization")) > 0 {
		if c.Request.Header.Get("Authorization")[0:5] != "Basic" {
			return UserName, Token, fmt.Errorf("only basic authorization scheme allowed")
		}
		Token = c.Request.Header.Get("Authorization")[6:]
	} else {
		return UserName, Token, fmt.Errorf("no Authorization token provided")
	}

	return UserName, Token, nil
}
