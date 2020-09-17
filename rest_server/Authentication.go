package main

import (
	"net/http"

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
		if c.Request.Header.Get("Authorization")[0:6] != "Bearer" {
			c.JSON(http.StatusUnauthorized, gin.H{"scheme_error:": c.Request.Header.Get("Authorization")[0:6]})
		}
		Token = c.Request.Header.Get("Authorization")[7:]
	} else {
		c.JSON(http.StatusUnauthorized, gin.H{"Authentication_error:": "No Authorization token provided"})
	}

	return UserName, Token, nil
}
