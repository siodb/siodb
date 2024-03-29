module github.com/siodb/siodb/rest_server

go 1.17

require (
	github.com/gin-gonic/gin v1.7.7
	golang.org/x/sys v0.0.0-20211007075335-d3039528d8ac
	google.golang.org/protobuf v1.27.1
	siodb.io/siodb/siodbproto v0.0.0
)

require (
	github.com/gin-contrib/sse v0.1.0 // indirect
	github.com/go-playground/locales v0.14.0 // indirect
	github.com/go-playground/universal-translator v0.18.0 // indirect
	github.com/go-playground/validator/v10 v10.9.0 // indirect
	github.com/golang/protobuf v1.5.2 // indirect
	github.com/json-iterator/go v1.1.12 // indirect
	github.com/leodido/go-urn v1.2.1 // indirect
	github.com/mattn/go-isatty v0.0.14 // indirect
	github.com/modern-go/concurrent v0.0.0-20180306012644-bacd9c7ef1dd // indirect
	github.com/modern-go/reflect2 v1.0.2 // indirect
	github.com/ugorji/go/codec v1.2.6 // indirect
	golang.org/x/crypto v0.0.0-20210921155107-089bfa567519 // indirect
	golang.org/x/text v0.3.6 // indirect
	gopkg.in/yaml.v2 v2.4.0 // indirect
)

replace siodb.io/siodb/siodbproto => ../build/all/generated-src/go-modules/siodb.io/siodb/siodbproto
