// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

package main

import (
	"SiodbIomgrProtocol"
	"fmt"
	"net/http"
	"time"

	"github.com/gin-gonic/gin"
)

func (restWorker RestWorker) postRows(c *gin.Context) {
	siodbLoggerPool.Debug("handler: postRows")
	restWorker.post(c, SiodbIomgrProtocol.DatabaseObjectType_ROW, c.Param("database_name")+"."+c.Param("table_name"), 0)
}

func (restWorker RestWorker) post(
	c *gin.Context, ObjectType SiodbIomgrProtocol.DatabaseObjectType, ObjectName string, ObjectId uint64) (err error) {

	start := time.Now()
	IOMgrConn, _ := IOMgrCPool.GetTrackedNetConn()
	defer CloseRequest(c, IOMgrConn, start)
	siodbLoggerPool.Debug("IOMgrConn: %v", IOMgrConn)

	var UserName, Token string
	if UserName, Token, err = loadAuthenticationData(c); err != nil {
		siodbLoggerPool.Error("%v", err)
		c.JSON(http.StatusUnauthorized, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	var requestID uint64
	if requestID, err = IOMgrConn.writeIOMgrRequest(
		SiodbIomgrProtocol.RestVerb_POST, ObjectType, UserName, Token, ObjectName, ObjectId); err != nil {
		siodbLoggerPool.Error("%v", err)
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	if err := IOMgrConn.readIOMgrResponse(requestID); err != nil {
		siodbLoggerPool.Error("%v", err)
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	// Write Payload
	if err := IOMgrConn.writeJSONPayload(requestID, c); err != nil {
		siodbLoggerPool.Error("%v", err)
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	if err := IOMgrConn.readIOMgrResponse(requestID); err != nil {
		siodbLoggerPool.Error("%v", err)
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	// Read and stream chunked JSON
	if err := IOMgrConn.readChunkedJSON(c); err != nil {
		siodbLoggerPool.Error("%v", err)
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	return nil
}
