// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

package main

import (
	"SiodbIomgrProtocol"
	"fmt"
	"net/http"
	"strconv"
	"time"

	"github.com/gin-gonic/gin"
)

func (restWorker RestWorker) getDatabases(c *gin.Context) {

	siodbLoggerPool.Debug("getDatabases")
	restWorker.get(c, SiodbIomgrProtocol.DatabaseObjectType_DATABASE, "", 0)
}

func (restWorker RestWorker) getTables(c *gin.Context) {

	siodbLoggerPool.Debug("getTables")
	restWorker.get(c, SiodbIomgrProtocol.DatabaseObjectType_TABLE, c.Param("database_name"), 0)
}

func (restWorker RestWorker) getRows(c *gin.Context) {

	siodbLoggerPool.Debug("getRows")
	restWorker.get(c, SiodbIomgrProtocol.DatabaseObjectType_ROW, c.Param("database_name")+"."+c.Param("table_name"), 0)
}

func (restWorker RestWorker) getRow(c *gin.Context) {

	siodbLoggerPool.Debug("handler: getRow")
	var rowID uint64
	var err error
	if rowID, err = strconv.ParseUint(c.Param("row_id"), 10, 64); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"Error:": "Invalid row_id"})
		siodbLoggerPool.Error("%v", err)
	} else {
		restWorker.get(c, SiodbIomgrProtocol.DatabaseObjectType_ROW, c.Param("database_name")+"."+c.Param("table_name"), rowID)
	}
}

func (restWorker RestWorker) get(
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
		SiodbIomgrProtocol.RestVerb_GET, ObjectType, UserName, Token, ObjectName, ObjectId); err != nil {
		siodbLoggerPool.Error("%v", err)
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	if err := IOMgrConn.readIOMgrResponse(requestID); err != nil {
		siodbLoggerPool.Error("%v", err)
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	} else {
		// Read and stream chunked JSON
		if err := IOMgrConn.readChunkedJSON(c); err != nil {
			siodbLoggerPool.Error("%v", err)
			c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
			return err
		}
	}

	return nil
}
