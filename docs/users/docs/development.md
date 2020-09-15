# Developer's Guide

## Drivers

Here is the list of currently supported drivers:

| Language | Link                                                 |
| -------- | :--------------------------------------------------- |
| Golang   | [Github](https://github.com/siodb/siodb-go-driver)   |
| Rust     | [Github](https://github.com/siodb/siodb-rust-driver) |
| Java     | [Github](https://github.com/siodb/siodb-java-driver) |
| Python   | [Github](https://github.com/siodb/siodb-python-driver) |
| C++      | [Github](https://github.com/siodb/siodb-cxx-driver) |
| .NET     | [Github](https://github.com/siodb/Siodb.EntityFrameworkCore.Siodb) |

## Supported data types

| Data type                | Available     | Description                                              |
| ------------------------ |:--------------|:---------------------------------------------------------|
| BOOLEAN                  | not yet       | SQL99 BOOLEAN                                            |
| TINYINT                  | yes           | SQL99 NUMERIC: 8-bit signed integer                      |
| TINYUINT                 | yes           | SQL99 NUMERIC: 8-bit unsigned integer                    |
| SMALLINT                 | yes           | SQL99 NUMERIC: 16-bit signed integer                     |
| SMALLUINT                | yes           | SQL99 NUMERIC: 16-bit unsigned integer                   |
| INT                      | yes           | SQL99 NUMERIC: 32-bit signed integer                     |
| UINT                     | yes           | SQL99 NUMERIC: 32-bit unsigned integer                   |
| BIGINT                   | yes           | SQL99 NUMERIC: 64-bit signed integer                     |
| BIGUINT                  | yes           | SQL99 NUMERIC: 64-bit unsigned integer                   |
| FLOAT                    | yes           | SQL99 FLOAT: 32-bit IEEE-754 floating-point number       |
| DOUBLE                   | yes           | SQL99 REAL/DOUBLE: 64-bit IEEE-754 floating-point number |
| TIMESTAMP                | yes           | SQL99 TIMESTAMP: Data and time                           |
| TIMESTAMP WITH TIME ZONE | not yet       | SQL99 TIMESTAMP WITH TIME ZONE                           |
| DATE                     | not yet       | SQL99 DATE: Date only                                    |
| TIME                     | not yet       | SQL99 TIME: Time only                                    |
| TIME WITH TIME ZONE      | not yet       | SQL99 TIME WITH TIME ZONE: ime with time zone            |
| TEXT                     | yes           | SQL99 CHAR, VARCHAR, CLOB: Textual data with UTF-8 enc.  |
| BLOB                     | yes           | SQL99 BLOB: Binary data of unlimited length              |
| JSON                     | not yet       | JSON                                                     |
| UUID                     | not yet       | UUID (128 bits)                                          |
| XML                      | not yet       | SQL99 XML                                                |

### TIMESTAMP

A timestamp with the nanosecond precision. You can use:

- CURRENT_TIMESTAMP
- A time string:
  - YYYY-MM-DD
  - YYYY-MM-DD HH:MM:SS
  - YYYY-MM-DD HH:MM:SS.SSS
  - YYYY-MM-DD HH:MM:SS.SSSSSSSSSS
