// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

package main

import (
	"fmt"
	"time"

	"github.com/gin-gonic/gin"
)

type restWorker struct {
	ginEngine *gin.Engine
	Port      uint32
}

func CORSMiddleware() gin.HandlerFunc {
	return func(c *gin.Context) {

		c.Header("Access-Control-Allow-Origin", "*")
		c.Header("Access-Control-Allow-Credentials", "true")
		c.Header("Access-Control-Allow-Headers", "Content-Type")
		c.Header("Access-Control-Allow-Methods", "GET, POST, PATCH, PUT, DELETE")

	}
}

func (worker *restWorker) CreateRouter(Port uint32) (err error) {
	worker.Port = Port
	worker.ginEngine = gin.New()
	worker.ginEngine.Use(CORSMiddleware())

	// GET
	worker.ginEngine.GET("/databases", worker.getDatabases)
	worker.ginEngine.GET("/databases/:database_name/tables", worker.getTables)
	worker.ginEngine.GET("/databases/:database_name/tables/:table_name/rows", worker.getRows)
	worker.ginEngine.GET("/databases/:database_name/tables/:table_name/rows/:row_id", worker.getRow)
	worker.ginEngine.GET("/query", worker.getSqlQuery)

	// POST
	worker.ginEngine.POST("/databases/:database_name/tables/:table_name/rows", worker.postRows)

	// PUT
	worker.ginEngine.PUT("/databases/:database_name/tables/:table_name/rows/:row_id", worker.patchRow)

	// PATCH
	worker.ginEngine.PATCH("/databases/:database_name/tables/:table_name/rows/:row_id", worker.patchRow)

	// DELETE
	worker.ginEngine.DELETE("/databases/:database_name/tables/:table_name/rows/:row_id", worker.deleteRow)

	return nil
}

func (worker *restWorker) StartHTTPRouter() (err error) {
	err = worker.ginEngine.Run(":" + fmt.Sprintf("%v", worker.Port))
	return err
}

func (worker *restWorker) StartHTTPSRouter(TLSCertificate string, TLSPrivateKey string) (err error) {

	err = worker.ginEngine.RunTLS(":"+fmt.Sprintf("%v", worker.Port), TLSCertificate, TLSPrivateKey)
	return err
}

func closeRequest(c *gin.Context, ioMgrConn *ioMgrConnection, start time.Time) {
	if r := recover(); r != nil {
		log.Debug("Recovered from: %v", r)
		ioMgrConn.Close()
		ioMgrConn.trackedNetConn.Conn = nil
	}
	ioMgrCPool.ReturnTrackedNetConn(ioMgrConn)
	log.Debug("ioMgrConn: %v", ioMgrConn)
	log.LogRequest(c, time.Since(start))
}
