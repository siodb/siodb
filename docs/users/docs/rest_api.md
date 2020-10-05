# REST API

## Authentication

The authentication throught REST is done with [HTTP Basic authentication](https://tools.ietf.org/html/rfc2617).
The username must a user in the Siodb instance and the password must be a token that belongs to this user.
The URL to the REST API can be:

`https://<USER_NAME>:<USER_TOKEN>@<SIODB_SERVER>:<SIODB_INSTANCE_PORT>/<PATH>`

### User and Token

You can generate a token for a user using SQL command:

```sql
ALTER USER <user_name> ADD TOKEN <token_name>;
```

Generated tokens are guaranteed at least to be unique among all currently existing tokens for the
mentioned used. Generated token is returned in the `DatabaseEngineReponse.freetext_message` in the
following format: `token: xxxxxxxx...xx`, where `xx...` are hexadecimal digits. There must be even
number of hexadecimal digits. Token is not stored in the database in this cleartext form, and if
token lost there is no way to renew it. In such case new token should be generated.

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

### Example

```bash
curl \
-H "Content-Type: application/json" \
https://root:0929b8e2f86a9c6c379a96cd8005072a09caf40affdf9b56098858f3d7188a1ab22bfcb4287b30b25aa1e7e8291a5605dc9b135ba2414e2c470a8ed7705d74d3@localhost:50443/databases/sys/tables/sys_tables/rows`
```

## Paths

### Databases

Action on all databases on the target instance.

#### URL structure

`https://<USER_NAME>:<USER_TOKEN>@localhost:50443/databases`

#### GET

**Description:** TODO.

**body:** none.

**Response:** TODO.

#### POST

**Description:** Not yet available.

#### PUT

**Description:** Not yet available.

#### PATH

**Description:** Not yet available.

#### DELETE

**Description:** Not yet available.

### Tables

Action on all tables from a database identified by `<DATABASE_NAME>`.

#### URL structure

`https://<USER_NAME>:<USER_TOKEN>@localhost:50443/databases/<DATABASE_NAME>/tables`

#### GET

**Description:** TODO.

**body:** none.

**Response:** TODO.

#### POST

**Description:** Not yet available.

#### PUT

**Description:** Not yet available.

#### PATH

**Description:** Not yet available.

#### DELETE

**Description:** Not yet available.

### Rows

#### Description

Action on all rows from a table identified by `<TABLE_NAME>`.

#### URL structure

`https://<USER_NAME>:<USER_TOKEN>@localhost:50443/databases/<DATABASE_NAME>/tables/<TABLE_NAME>/rows`

#### GET

**Description:** TODO.

**body:** none.

**Response:**

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

#### POST

**Description:** TODO.

**body:** A JSON dictionary encoded with UTF-8 character set.

```json
[
    { "col1": "Val1", "col2": "Val2", ...},
    { "col1": "Val1", "col2": "Val2", ...},
    ...
]
```

**Response:**

```json
TODO
```

#### PUT

**Description:** TODO.

**body:** none.

**Response:** TODO.

#### PATH

**Description:** TODO.

**body:** none.

**Response:** TODO.

#### DELETE

**Description:** TODO.

**body:** none.

**Response:** TODO.

### Row

#### Description

Action on one row identified by `<TRID>` from a table identified by `<TABLE_NAME>`.

#### URL structure

`https://<USER_NAME>:<USER_TOKEN>@localhost:50443/databases/<DATABASE_NAME>/tables/<TABLE_NAME>/rows/<TRID>`

#### GET

**Description:** TODO.

**body:** none.

**Response:** TODO.

#### POST

**Description:** TODO.

**body:** none.

**Response:** TODO.

#### PUT

**Description:** TODO.

**body:** none.

**Response:** TODO.

#### PATH

**Description:** TODO.

**body:** none.

**Response:** TODO.

#### DELETE

**Description:** TODO.

**body:** none.

**Response:** TODO.
