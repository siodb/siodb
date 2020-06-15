# REST server

Siodb must provide an integrated REST server that can work in parallel with the SQL front-end.
The server will be a distinct process called `siodb_rest_server`.
There will be new parameters to indicate the ports numbers on which the process will
listen:

- `rest_server.ipv4_http_port`. Default port `50080`. `0` means do not listen.
- `rest_server.ipv4_https_port`. Default port `50443`. `0` means do not listen.
- `rest_server.ipv6_http_port`. Default port `50080`. `0` means do not listen.
- `rest_server.ipv6_https_port`. Default port `50443`. `0` means do not listen.

The process will convert the REST queries into the IOMgr engine protobuf
message (From REST to SQL).

It would only work per table and allow users to push/retrieve data through REST.

## Third-party libraries

Siodb will use Oat++ (https://github.com/oatpp/oatpp) for the implementation of the REST server
in the `siodb_rest_server` process.

Also, we would leverage the JSON API from Protobuf as much as possible for conversion between
message to JSON. E.g. when sending the IOMgr response to the client.

## Authentication

Siodb must provide a new table `sys_user_tokens` linked with `sys_users.trid`. In that table
the columns `name` (text) and `value` (text) must store the token information for each user.
There must also be a column `description` (text) as Siodb allows descriptions for all object types.

It is then possible to generate a token with an SQL command:

```sql
ALTER USER <user_name> CREATE TOKEN '<token_name>';
```

It is possible to delete a token with this SQL commands:

```sql
ALTER USER <user_name> DROP TOKEN '<token_name>';
```

A user must provide the Siodb user name plus a valid token to access and manipulate the data:

```curl --user "Siodb_user_name"  https://siodb-srv:50443/<path>```

## REST Paths (for the first version)

### GET

`/databases/<database_name>/tables/<table_name>/rows`: Return all rows.
`/databases/<database_name>/tables/<table_name>/rows/<trid>`: returns row with specified trid:

- Return the JSON Response:

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

`/databases/<database_name>/tables/<table_name>/rows`: Create a new row

- Body: the name of the columns and their values: `{ "col1": "ValCol1", "col2": "ValCol2", ...}`.
- Create the row with specified trid.
- Return the JSON Response.

### PUT

`/databases/<database_name>/tables/<table_name>/rows/<trid>`:

- Body: the name of the columns and their values: `{ "col1": "ValCol1", "col2": "ValCol2", ...}`.
- Update the row with specified trid.
- Return the JSON Response.

### DELETE

`/databases/<database_name>/tables/<table_name>/rows/<trid>`:

- Body: none.
- Delete the row with specified trid.
- Return the JSON Response.

### POST/DELETE/UPDATE success response

```json
// HTTP OK = 200
{
    "status": 200,
    "affected_rows": <affected_rows>,
    "trid": [ "<trid_1>", "...", "<trid_N>" ]
}
```

### Error response

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

1. A client sends a query to the server:
`curl --user "Siodb_user_name"  https://siodb-srv:50443/<path>`
2. `siodb_rest_server` receives the request and queries the IOMgr to see if the token is valid.
    - If valid: it goes to step 3
    - If not valid, it return the message `Status::CODE_401, "Unauthorized"`
3. `siodb_rest_server` receives the request.
4. `siodb_rest_server` converts the request into a `DatabaseEngineRestRequest`.
5. `siodb_rest_server` sends the command to the IOMgr.
6. IOMgr get the `DatabaseEngineRestRequest`, process the JSON and send the `DatabaseEngineResponse`
along with the raw data as it would for a standard SQL.
7. `siodb_rest_server` writes into a JSON the response converted from `DatabaseEngineResponse`.
Send the HTTP 1.1 header `Transfer-Encoding: chunked` + the JSON response.
8. `siodb_rest_server` receives raw data and forms the resultset in a JSON form.
Send the HTTP 1.1 chunks, chunked per `N`* rows. Stop when raw data lenth == 0.
9. `siodb_rest_server` sends the JSON response to the client.

NOTE: All values from IOMgr would be converted into UTF-8 JSON.

* N is configurable with the parameter `rest_server.rows_per_http_chunk`.

### POST, PUT, DELETE

1. A client sends a query to the server:
`curl --user "Siodb_user_name"  https://siodb-srv:50443/<path>`
2. `siodb_rest_server` receives the request and queries the IOMgr to see if the token is valid.
    - If valid: it goes to step 3
    - If not valid, it return the message `Status::CODE_401, "Unauthorized"`
3. `siodb_rest_server` receives the request.
4. `siodb_rest_server` converts the request into a `DatabaseEngineRestRequest`.
5. `siodb_rest_server` sends the command to the IOMgr.
6. IOMgr get the `DatabaseEngineRestRequest`, process the JSON and send the `DatabaseEngineResponse`.
7. `siodb_rest_server` writes into a JSON the response converted from `DatabaseEngineResponse`.
8. `siodb_rest_server` sends the JSON response to the client.

## Security

The REST server in the `siodb_rest_server` process must support HTTPS. For HTTPS, it should use
the following parameters:

```ini
rest_server.enable_encryption = yes
rest_server.tls_certificate = /etc/siodb/instances/siodb/cert.pem
rest_server.tls_certificate_chain = /etc/siodb/instances/siodb/certChain.pem
rest_server.tls_private_key = /etc/siodb/instances/siodb/key.pem
```
