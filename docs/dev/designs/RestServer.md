# REST Server

## Overview

Siodb must to provide an integrated REST Server that can work in parallel with the SQL front-end.
It would only work per table and allow users to push/retrieve data through REST. The server will
operate as a distinct process called `siodb_rest_server`. The server will convert REST queries
into the IOMgr engine requests, send them to IOMgr, receive response, convert it to JSON and send
back to HTTP client. REST Server will support both HTTP and HTTPS protocols.

## Authentication

Siodb must provide a new table `sys_user_tokens` with following columns:
| Column Name          | Data type | Mandatory | Description                |
| -------------------- | --------- | --------- | -------------------------- |
| TRID                 | UINT64    | Yes       | Token ID                   |
| USER_ID              | UINT64    | Yes       | User ID                    |
| NAME                 | TEXT      | Yes       | Token name                 |
| VALUE                | BINARY    | Yes       | Token value                |
| EXPIRATION_TIMESTAMP | TIMESTAMP | No        | Token expiration timestamp |
| DESCRIPTION          | TEXT      | No        | Token description          |

New token is generated using SQL command:

```sql
ALTER USER <user_name> ADD TOKEN <token_name>;
```

Generated tokens are guaranteed at least to be unique among all currently existing tokens for the
mentioned used. Generated token is returned in the `DatabaseEngineReponse.freetext_message` in the
following format: `token: xxxxxxxx...xx`, where `xx...` are hexadecimal digits. There must be even
number of hexadecimal digits. Token is not stored in the database in this cleartext form, and if
token lost there is no way to restore it. In such case new token should be generated.

Token can be assigned expiration time:

```sql
ALTER USER <user_name> ADD TOKEN <token_name> WITH EXPIRATION_TIME='yyyy-mm-dd hh:mm:ss';
```

It is possible to add token with supplied value:

```sql
ALTER USER <user_name> ADD TOKEN <token_name> x'xxxx...xx';
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

- Body: array of objects with column name-value pairs. Each element in the array
  designates separate row.

```json
[
    { "col1": "Val1", "col2": "Val2", ...},
    { "col1": "Val1", "col2": "Val2", ...},
    ...
]
```

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

### GET, DELETE Requests

1. A client sends a query to the server, for example:
`curl -H "Authorization: Basic xxxx" https://siodb-srv:50443/request-path`
2. REST Server converts the request into message `DatabaseEngineRestRequest`.
3. REST Server sends message `DatabaseEngineRestRequest` to the IOMgr.
4. IOMgr receives the message `DatabaseEngineRestRequest`, evaluates user name and token,
   processes request and sends message `DatabaseEngineResponse` and, if applicable,
   json data after it in IOMgr chunked format.
5. REST server receives message `DatabaseEngineResponse` from IOMgr.
6. If `DatabaseEngineResponse` contains error messages, REST Server generates
   JSON with error status and sends only it.
7. Otherwise:
   - REST Server sends HTTP 1.1 header `Transfer-Encoding: chunked`.
   - REST Server receives JSON from IOMgr in the chunked format and forwards it
     to the client in the HTTP 1.1 chunks, chunked per N bytes.
   - Finally REST Server sends zero-length chunk to indicate end of data.

NOTE: All values from IOMgr would be converted into UTF-8 JSON.

### POST, PATCH Requests

1. A client sends a query to the server:
`curl -H "Authorization: Basic xxxx" https://siodb-srv:50443/<path>`
2. REST Server receives the request headers, creates and sends message
   `DatabaseEngineRestRequest` to IOMgr.
3. IOMgr receives the `DatabaseEngineRestRequest`, evaluates user name and token,
   and sends the `DatabaseEngineResponse`.
4. REST server receives `DatabaseEngineResponse`.
5. If `DatabaseEngineResponse` contains error messages, REST Server:
   - Receives and skips request body
   - Generates JSON with error status (i.e. `Status::CODE_401, "Unauthorized"`)
     and sends only it.
6. REST Server receives request body and forwards it to the IOMgr in the IOMgr
   chunked format.
7. IOMgr receives JSON data and parses it.
8. If JSON contains errors, IOMgr creates message `DatabaseEngineResponse`
   with errors and sends it back.
9. IOMgr processes JSON, performs operations and forms reponse payload.
10. IOMgr creates message `DatabaseEngineResponse` and sends it back.
11. REST Server receives message `DatabaseEngineResponse`.
12. If `DatabaseEngineResponse` contains error messages, REST Server forms JSON
    with appropriate status and sends only it.
13. Otherwise:
    - REST Server sends HTTP 1.1 header `Transfer-Encoding: chunked`.
    - REST Server receives JSON from IOMgr in the chunked format and forwards it
      to the client in the HTTP 1.1 chunks, chunked per N bytes.
    - Finally REST Server sends zero-length chunk to indicate end of data.

## IOMgr Chunked Data Format

IOMgr chunked data format consists of the following elements:

1. Chunk length: `varuint32` number.
2. Chunk content.
3. (1) and (2) are repeated as needed.
4. Zero byte as chunk length indicates end of chunked data.

## Configuration Parameters

- `enable_rest_server` - enables or disables REST Server service.
- `rest_server.ipv4_http_port` - IPv4 HTTP port number. Default value `50080`.
  Zero value means do not listen.
- `rest_server.ipv4_https_port` - IPv4 HTTPS port number. Default value `50443`.
  Zero value means do not listen.
- `rest_server.ipv6_http_port` - IPv6 HTTP port number. Default value `0`.
  Zero value means do not listen.
- `rest_server.ipv6_https_port` - IPv6 HTTPS port number. Default value `0`.
  Zero value means do not listen.
- `rest_server.tls_certificate` - path to the TLS certificate file.
- `rest_server.tls_certificate_chain` - path to the TLS certificate chain file.
- `rest_server.tls_private_key` - path to the TLS private key file.
- `rest_server.chunk_size` - HTTP chunk size in bytes.
   Suffixes `k`, `K`, `m`, `M` change units to kilobytes and megabytes
   respectively.
- `iomgr.rest.ipv4_port` - IPv4 REST protocol poort. Defaulr value `50002`.
  `0` means do not listen.
- `iomgr.rest.ipv6_port` - IPv6 REST protocol poort. Default value `0`.
  Zero value means do not listen.
- `iomgr.max_json_payload_size` - Maximum JSON payload size in kilobytes.
  Suffixes `k`, `K`, `m`, `M`, `g`, `G` change units to kilobytes, megabytes
  and gigabytes respectively.
