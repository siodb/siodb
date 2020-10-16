# REST API

The REST API of Siodb allows you to push data into tables and retrieve data from tables over the HTTP
protocol.

## Authentication

See the [Authentication page](authentication.md).

## REST Paths

### Databases

Action on all databases on the target instance.

#### URL structure

`https://<USER_NAME>:<USER_TOKEN>@localhost:50443/databases`

#### GET

**Description:** Retrieve the name of the databases in the Siodb instance.

**body:** none.

**Response:**

```json
{
    "status": 200,
    "rows" : [
        {
            "name":"DATABASE_1"
        },
        {
            "name":"DATABASE_2"
        }
        // more database names
    ]
}
```

### Tables

Action on all tables from a database identified by `<DATABASE_NAME>`.

#### URL structure

`https://<USER_NAME>:<USER_TOKEN>@localhost:50443/databases/<DATABASE_NAME>/tables`

#### GET

**Description:** Retrieve the name of tables available in the database <DATABASE_NAME>.

**body:** none.

**Response:**

```json
{
    "status": 200,
    "rows" : [
        {
            "name":"TABLE_1"
        },
        {
            "name":"TABLE_2"
        },
        // more table names
    ]
}
```

### Rows

#### Description

Action on all rows from a table identified by `<TABLE_NAME>`.

#### URL structure

`https://<USER_NAME>:<USER_TOKEN>@localhost:50443/databases/<DATABASE_NAME>/tables/<TABLE_NAME>/rows`

#### GET

**Description:** Retrieve the rows from the table <DATABASE_NAME>.<TABLE_NAME>.

**body:** none.

**Response:**

```json
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

**Description:** Create one or more rows from a payload.

**body:** An array of objects with column name-value pairs. Each element in the array
designates a separate row.

```json
[
    { "col1": "Val1", "col2": "Val2", ...},
    { "col1": "Val1", "col2": "Val2", ...},
    ...
]
```

**Response:**

```json
{
    "status": 200,
    "affectedRowCount": <number of affected rows>,
    "trids" : [
        <1st Table Row ID created>,
        <2nd Table Row ID created>
        // more table row IDs
    ]
}
```

**Example**:

```bash
curl -X POST \
-d @data.json \
http://root:c3c2b7d94797a28b9ca1d38d11b785b438b673c1a759ed88d45a44879a3d387fe99c9c8c0b90205f45fcf87afe2d12078bf8a906f3e000444cc1a5fd23e053e3@localhost:50080/databases/test/tables/test/rows
```

```json
{
	"status": 200,
	"affectedRowCount": 6,
	"trids": [5001, 5002, 5003, 5004, 5005, 5006]
}
```

### Row

#### Description

Action on one row identified by `<TRID>` from a table identified by `<TABLE_NAME>`.

#### URL structure

`https://<USER_NAME>:<USER_TOKEN>@localhost:50443/databases/<DATABASE_NAME>/tables/<TABLE_NAME>/rows/<TRID>`

#### GET

**Description:** Retrieve a row from the table <DATABASE_NAME>.<TABLE_NAME> which has
the table row ID == <TRID>.

**body:** none.

**Response:**

```json
{
    "status": 200,
    "rows" : [
        {
            "col1": "dataCol1Row1",
            "col2": "dataCol2Row1"
            // more columns
        }
    ]
}
```

#### PUT

**Description:** Update with the payload content the row identified by the
table row ID == <TRID>.

**body:** An array of objects with column name-value pairs. Each element in the array
designates a separate row.

```json
[
    { "col1": "Val1", "col2": "Val2", ...},
    { "col1": "Val1", "col2": "Val2", ...},
    ...
]
```

**Response:**

```json
{
    "status": 200,
    "affectedRowCount": <number of affected rows>,
    "trids" : [
        <1st Table Row ID update>
    ]
}
```

#### PATCH

**Description:** Update with the payload content the row identified by the
table row ID == <TRID>.

**body:** An array of objects with column name-value pairs. Each element in the array
designates a separate row.

```json
[
    { "col1": "Val1", "col2": "Val2", ...},
    { "col1": "Val1", "col2": "Val2", ...},
    ...
]
```

**Response:**

```json
{
    "status": 200,
    "affectedRowCount": <number of affected rows>,
    "trids" : [
        <1st Table Row ID updated>
    ]
}
```

#### DELETE

**Description:** Delete the row identified by the table row ID == <TRID>.

**body:** none.

**Response:**

```json
{
    "status": 200,
    "affectedRowCount": <number of affected rows>,
    "trids" : [
        <1st Table Row ID deleted>
    ]
}
```
