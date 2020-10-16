# System Tables

This page reference the tables that Siodb manages to function.
Siodb adds an internal column `TRID`. Siodb uses that column as a primary key for all
rows in a table. Siodb increment the `TRID` at each `INSERT` operation.
The number is 64-bit unsigned integer that can grow up to 18446744073709551615.

## SYS_TABLES

Stores information about known tables.

- TRID: The unique ID of the row
- TYPE: Table storage type
- NAME: Table name
- FIRST_USER_TRID: First user record row identifier
- CURRENT_COLUMN_SET_ID: Current column set
- DESCRIPTION: Table description

## SYS_DUMMY

Helper table for computing constant expressions.

- TRID: The unique ID of the row
- DUMMY: Dummy column

## SYS_COLUMN_SETS

Stores information about column sets.

- TRID: The unique ID of the row
- TABLE_ID: Table identifier
- COLUMN_COUNT: Number of columns in this column set

## SYS_COLUMNS

Stores information about table columns.

- TRID: The unique ID of the row
- TABLE_ID: Table identifier
- DATA_TYPE: Column data type
- NAME: Column name
- STATE: Column state
- BLOCK_DATA_AREA_SIZE: Data area size in the block file
- DESCRIPTION: Column description

## SYS_COLUMN_DEFS

Stores information about column definitions.

- TRID: The unique ID of the row
- COLUMN_ID: Column identifier
- CONSTRAINT_COUNT: Number of constraints associated with this column definition

## SYS_COLUMN_SET_COLUMNS

Stores information about inclusion of column definitions into column sets.

- TRID: The unique ID of the row
- COLUMN_SET_ID: Column set identifier
- COLUMN_DEF_ID: Associated column definition identifier

## SYS_CONSTRAINT_DEFS

Stores information about unique constraint definitions.

- TRID: The unique ID of the row
- TYPE: Constraint type
- EXPR: Constraint expression

## SYS_CONSTRAINTS

Stores information about constraints.

- TRID: The unique ID of the row
- NAME: Constraint name
- STATE: Constraint state
- TABLE_ID: Table identifier, to which this constraint belongs
- COLUMN_ID: Column identifier, to which this constraint belongs
- DEF_ID: Constraint definition identifier
- DESCRIPTION: Constraint description

## SYS_COLUMN_DEF_CONSTRAINTS

Stores information about constraints associated with column definitions.

- TRID: The unique ID of the row
- COLUMN_DEF_ID: Column definition identifier
- CONSTRAINT_ID: Associated constraint identifier

## SYS_INDICES

Stores information about indices.

- TRID: The unique ID of the row
- TYPE: Index type
- UNIQUE: Indication that index is unique
- NAME: Index name
- TABLE_ID: Table identifier, to which index applies
- DATA_FILE_SIZE: Data file size
- DESCRIPTION: Index description

## SYS_INDEX_COLUMNS

Stores information about indexed columns.

- TRID: The unique ID of the row
- INDEX_ID: Index identifier
- COLUMN_DEF_ID: Associated column defintion identifier
- SORT_DESC: Indication of descending sort order by this column

## SYS_USERS

Stores information about users.

- TRID: The unique ID of the row
- NAME: User name
- REAL_NAME: User's real name
- STATE: User state
- DESCRIPTION: User description

## SYS_USER_ACCESS_KEYS

Stores information about user's access keys.

- TRID: The unique ID of the row
- USER_ID: User identifier
- NAME: Access key name
- TEXT: Access key text
- STATE: Access key state
- DESCRIPTION: Access key description

## SYS_DATABASES

Stores information about known databases.

- TRID: The unique ID of the row
- UUID: Database UUID
- NAME: Database name
- CIPHER_ID: Cipher identifier
- DESCRIPTION: Database description
- MAX_TABLES: Maximum number of tables

## SYS_USER_PERMISSIONS

Stores information about user permissions.

- TRID: The unique ID of the row
- USER_ID: User identifier
- DATABASE_ID: Database identifier
- OBJECT_TYPE: Database object type
- OBJECT_ID: Database object identifier
- PERMISSIONS: Permission mask
- GRANT_OPTIONS: Grant option mask

## SYS_USER_TOKENS

Stores authentication tokens.

- TRID: The unique ID of the row
- USER_ID: User identifier
- NAME: Token name
- VALUE: Token value
- EXPIRATION_TIMESTAMP: Token expiration timestamp
- DESCRIPTION: Token description
