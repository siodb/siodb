# REST server

## Overview

Siodb must to provide an integrated REST server that can work in parallel with theSQL front-end.
It would only work per table and allow users to push/retrieve data through REST. The server will
operate as a distinct process called `siodb_rest_server`. The server will convert REST queries
into the IOMgr engine requests, send them to IOMgr, receive response, convert it to JSON and send
back to HTTP client. REST server will support both HTTP and HTTPS protocols.

## Authentication

Siodb must provide a new table `sys_user_tokens` with following columns:
|Column Name|Data type|Mandatory|Description|
|---|---|---|---|
|TRID|UINT64|Yes|Token ID|
|USER_ID|UINT64|Yes|User ID|
|NAME|TEXT|Yes|Token name|
|VALUE|BINARY|Yes|Token value|
|EXPIRATION_TIMESTAMP|TIMESTAMP|No|Token expiration timestamp|
|DESCRIPTION|TEXT|No|Token description|

New token is generated using SQL command:

```sql
ALTER USER <user_name> CREATE TOKEN <token_name>;
```

Generated tokens are guaranteed at least to be unique among all currently existing tokens for the
mentioned used. Generated token is returned in the `DatabaseEngineReponse.freetext_message` in the
following format: `token: xxxxxxxx...xx`, where `xx...` are hexadecimal digits. There must be even
number of hexadecimal digits. Token is not stored in the database in this cleartext form, and if
token lost there is no way to renew it. In such case new token should be generated.

Token can be assigned expiration time:

```sql
ALTER USER <user_name> CREATE TOKEN <token_name> WITH EXPIRATION_TIME='yyyy-mm-dd hh:mm:ss';
```

It is possible to add token with supplied value:

```sql
ALTER USER <user_name> CREATE TOKEN <token_name> x'xxxx...xx';
```

Supplied value must be unique among existing tokens for the designated user,
otherwise token will not be created.

It is possible to delete a token with following SQL commands:

```sql
ALTER USER <user_name> DROP TOKEN <token_name>;
```

Client must provide a valid token to access and manipulate the data. For example:

```curl -H "Authorization: Basic xxxx" https://siodb-srv:50443/<path>```

where `xxxx` is base64-encoded pair `user name:user token`.

## REST Paths (for the first version)

### GET

URLs:

- `/databases/<database_name>/tables/<table_name>/rows`: for all rows.
- `/databases/<database_name>/tables/<table_name>/rows/<trid>`: for a specific row.

Action:

- Find all matching rows.

Result:

- Returns JSON response:

```json
// HTTP OK = 200
{
    "status": 200,
    "rows" : [
        {
            "col1": "dataCol1Row1",
            "col2": "dataCol2Row1"
            // more columns
        },
        {
            // next row
        },
        // more rows
    ]
}
```

### POST

URL:

- `/databases/<database_name>/tables/<table_name>/rows`

Content:

- Body: column name-value pairs: `{ "col1": "Val1", "col2": "Val2", ...}`.

Action:

- Creates one or more new rows.

Result:

- Returns JSON response.

### PATCH

URL:

- `/databases/<database_name>/tables/<table_name>/rows/<trid>`

Content:

- Body: column name-value pairs: `{ "col1": "Val1", "col2": "Val2", ...}`.

Actions:

- Updates row with specified TRID.

Result:

- Return JSON response.

### DELETE

URL:

- `/databases/<database_name>/tables/<table_name>/rows/<trid>`

Content:

- None.

Actions:

- Deletes row with specified TRID.

Result:

- Return JSON response.

### POST/PATCH/DELETE Success Response

```json
// HTTP OK = 200
{
    "status": 200,
    "affectedRowCount": <affected_rows>,
    "trids": [ <trid_1>, ..., <trid_N> ]
}
```

### GET/POST/PATCH/DELETE Error Response

```json
// HTTP BAD_REQUEST = 400
{
    "status": 400,
    "errors" : [
        {
            "code": <siodb error code>,
            "message": "<siodb error message>"
        },
        {
            // next error mesage ....
        },
        // ... more errors
    ]
}
```

## Workflows

### GET

1. A client sends a query to the server, for example:
`curl -H "Authorization: Basic xxxx" https://siodb-srv:50443/request-path`
2. REST server receives the request headers and queries the IOMgr with `ValidateUserTokenRequest`
   to see if the token is valid.
    - If valid: continues to the step 3
    - If invalid: returns HTTP status `401 Unauthorized`
3. REST server receives the request body.
4. REST server converts the request into a `DatabaseEngineRestRequest`.
5. REST server sends the command to the IOMgr.
6. IOMgr receives the `DatabaseEngineRestRequest`, re-evaluates token,
   processes the JSON and sends the `DatabaseEngineResponse` and, if applicable, json data after it.
7. REST server converts `DatabaseEngineResponse` to JSON and sends the HTTP 1.1 header
   `Transfer-Encoding: chunked` and part of the JSON response.
8. REST server receives JSON and forwards it to the client in the HTTP 1.1 chunks,
   chunked per N bytes.
9. Finally REST server sends zero-length chunk to indicate end of data.

NOTE: All values from IOMgr would be converted into UTF-8 JSON.

### POST, PATCH, DELETE

1. A client sends a query to the server:
`curl -H "Authorization: Basic xxxx" https://siodb-srv:50443/<path>`
2. REST server receives the request headers and queries the IOMgr with `ValidateUserTokenRequest`
   to see if the token is valid.
    - If valid: it goes to step 3
    - If not valid, it return the message `Status::CODE_401, "Unauthorized"`
3. REST server receives the request.
4. REST server converts the request into a `DatabaseEngineRestRequest`.
5. REST server sends the command to the IOMgr.
6. IOMgr receives the request, re-evaluates token, processes the JSON and send the
   `DatabaseEngineResponse` and, if applicable, JSON data after it.
7. REST server writes into a JSON the response converted from `DatabaseEngineResponse`.
8. REST server sends the JSON response to the client along with JSON data from iomgr.

## Configuration Parameters

- `rest_server.ipv4_http_port` - IPv4 HTTP port number. Default value `50080`.
  `0` means do not listen.
- `rest_server.ipv4_https_port` - IPv4 HTTPS port number. Default value `50443`.
  `0` means do not listen.
- `rest_server.ipv6_http_port` - IPv6 HTTP port number. Default value `50080`.
  `0` means do not listen.
- `rest_server.ipv6_https_port` - IPv6 HTTPS port number. Default value `50443`.
  `0` means do not listen.
- `rest_server.tls_certificate` - path to the TLS certificate file.
- `rest_server.tls_certificate_chain` - path to the TLS certificate chain file.
- `rest_server.tls_private_key` - path to the TLS private key file.
- `rest_server.chunk_size` - HTTP chunk size in bytes. Suffixes `k`, `K`, `m`, `M` change units
  to kilobytes and megabytes respectively.
