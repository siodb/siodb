package main

import (
	"fmt"

	"github.com/gin-gonic/gin"
)

type RestWorker struct {
	ginEngine *gin.Engine
	Port      uint32
	RequestID uint64
}

func (restWorker *RestWorker) CreateRouter(Port uint32) (err error) {

	restWorker.Port = Port
	restWorker.ginEngine = gin.New()

	// Log at router level
	for _, w := range siodbLoggerPool.siodbLogger {
		restWorker.ginEngine.Use(gin.LoggerWithWriter(w.out))
	}
	restWorker.ginEngine.Use(gin.LoggerWithFormatter(func(param gin.LogFormatterParams) string {
		return siodbLoggerPool.GinFormattedOutput(param)
	}))

	// GET
	restWorker.ginEngine.GET("/databases", restWorker.getDatabases)
	restWorker.ginEngine.GET("/databases/:database_name/tables", restWorker.getTables)
	restWorker.ginEngine.GET("/databases/:database_name/tables/:table_name/rows", restWorker.getRows)
	restWorker.ginEngine.GET("/databases/:database_name/tables/:table_name/rows/:row_id", restWorker.getRow)

	// POST
	restWorker.ginEngine.POST("/databases/:database_name/tables/:table_name/rows", restWorker.postRows)

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
