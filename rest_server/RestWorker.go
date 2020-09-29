// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

package main

import (
	"fmt"

	"github.com/gin-gonic/gin"
)

type RestWorker struct {
	ginEngine *gin.Engine
	Port      uint32
}

func (restWorker *RestWorker) CreateRouter(Port uint32) (err error) {

	restWorker.Port = Port
	restWorker.ginEngine = gin.New()

	// GET
	restWorker.ginEngine.GET("/databases", restWorker.getDatabases)
	restWorker.ginEngine.GET("/databases/:database_name/tables", restWorker.getTables)
	restWorker.ginEngine.GET("/databases/:database_name/tables/:table_name/rows", restWorker.getRows)
	restWorker.ginEngine.GET("/databases/:database_name/tables/:table_name/rows/:row_id", restWorker.getRow)

	// POST
	restWorker.ginEngine.POST("/databases/:database_name/tables/:table_name/rows", restWorker.postRows)

	// PUT
	restWorker.ginEngine.PUT("/databases/:database_name/tables/:table_name/rows/:row_id", restWorker.patchRow)

	// PATCH
	restWorker.ginEngine.PATCH("/databases/:database_name/tables/:table_name/rows/:row_id", restWorker.patchRow)

	// DELETE
	restWorker.ginEngine.DELETE("/databases/:database_name/tables/:table_name/rows/:row_id", restWorker.deleteRow)

	return nil
}

func (restWorker *RestWorker) StartHTTPRouter() (err error) {

	err = restWorker.ginEngine.Run(":" + fmt.Sprintf("%v", restWorker.Port))
	return err
}

func (restWorker *RestWorker) StartHTTPSRouter(TLSCertificate string, TLSPrivateKey string) (err error) {

	err = restWorker.ginEngine.RunTLS(":"+fmt.Sprintf("%v", restWorker.Port), TLSCertificate, TLSPrivateKey)
	return err
}
