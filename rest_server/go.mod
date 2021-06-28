module github.com/siodb/siodb/rest_server

go 1.16

require (
	github.com/gin-gonic/gin v1.7.1
	golang.org/x/sys v0.0.0-20210415045647-66c3f260301c
	google.golang.org/protobuf v1.27.0
	siodb.io/siodb/siodbproto v0.0.0
)

replace siodb.io/siodb/siodbproto => ../build/all/generated-src/go-modules/siodb.io/siodb/siodbproto
