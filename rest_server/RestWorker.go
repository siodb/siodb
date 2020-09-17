package main

import (
	"fmt"

	"github.com/gin-gonic/gin"
)

type RestWorker struct {
	router    RestServerRouter
	RequestID uint64
}

func (restWorker *RestWorker) CreateRouter() (err error) {

	restWorker.router.ginEngine = gin.New()

	// Log at router level
	for _, w := range siodbLoggerPool.siodbLogger {
		restWorker.router.ginEngine.Use(gin.LoggerWithWriter(w.out))
	}
	restWorker.router.ginEngine.Use(gin.LoggerWithFormatter(func(param gin.LogFormatterParams) string {
		return siodbLoggerPool.GinFormattedOutput(param)
	}))

	// GET
	restWorker.router.ginEngine.GET("/databases", restWorker.getDatabases)
	restWorker.router.ginEngine.GET("/databases/:database_name/tables", restWorker.getTables)
	restWorker.router.ginEngine.GET("/databases/:database_name/tables/:table_name/rows", restWorker.getRows)
	restWorker.router.ginEngine.GET("/databases/:database_name/tables/:table_name/rows/:row_id", restWorker.getRow)

	// POST
	restWorker.router.ginEngine.POST("/databases/:database_name/tables/:table_name/rows", restWorker.postRows)

	return nil
}

func (restWorker *RestWorker) StartHTTPRouter() (err error) {

	err = restWorker.router.ginEngine.Run(":" + fmt.Sprintf("%v", restWorker.router.Port))
	return err

}

func (restWorker *RestWorker) StartHTTPSRouter(TLSCertificate string, TLSPrivateKey string) (err error) {

	err = restWorker.router.ginEngine.RunTLS(":"+fmt.Sprintf("%v", restWorker.router.Port), TLSCertificate, TLSPrivateKey)
	return err

}
