// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

package main

import (
	"fmt"
	"net/http"
	"strconv"
	"time"

	"github.com/gin-gonic/gin"

	"siodb.io/siodb/siodbproto"
)

func (worker restWorker) getDatabases(c *gin.Context) {
	log.Debug("getDatabases")
	worker.get(c, siodbproto.DatabaseObjectType_DATABASE, "", 0)
}

func (worker restWorker) getTables(c *gin.Context) {
	log.Debug("getTables")
	worker.get(c, siodbproto.DatabaseObjectType_TABLE, c.Param("database_name"), 0)
}

func (worker restWorker) getRows(c *gin.Context) {
	log.Debug("getRows")
	worker.get(c, siodbproto.DatabaseObjectType_ROW, c.Param("database_name")+"."+c.Param("table_name"), 0)
}

func (worker restWorker) getRow(c *gin.Context) {
	log.Debug("handler: getRow")
	var rowID uint64
	var err error
	if rowID, err = strconv.ParseUint(c.Param("row_id"), 10, 64); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"Error:": "Invalid row_id"})
		log.Error("%v", err)
	} else {
		worker.get(c, siodbproto.DatabaseObjectType_ROW, c.Param("database_name")+"."+c.Param("table_name"), rowID)
	}
}

func (worker restWorker) get(
	c *gin.Context, ObjectType siodbproto.DatabaseObjectType, ObjectName string, ObjectID uint64) (err error) {

	start := time.Now()
	ioMgrConn, _ := ioMgrCPool.GetTrackedNetConn()
	defer closeRequest(c, ioMgrConn, start)
	log.Debug("ioMgrConn: %v", ioMgrConn)

	var userName, token string
	if userName, token, err = loadAuthenticationData(c); err != nil {
		log.Error("%v", err)
		c.JSON(http.StatusUnauthorized, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	var requestID uint64
	if requestID, err = ioMgrConn.writeIOMgrRequest(
		siodbproto.RestVerb_GET, ObjectType, userName, token, ObjectName, ObjectID); err != nil {
		log.Error("%v", err)
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	if restStatusCode, err := ioMgrConn.readIOMgrResponse(requestID); err != nil {
		log.Error("%v", err)
		c.JSON(int(restStatusCode), gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	} else {
		c.Writer.WriteHeader(int(restStatusCode))
	}
	// Read and stream chunked JSON
	if err := ioMgrConn.readChunkedJSON(c); err != nil {
		log.Error("%v", err)
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	return nil
}
