-- Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
-- Use of this source code is governed by a license that can be found
-- in the LICENSE file.

create database testdb;

CREATE TABLE testdb.table_with_all_data_types (
    ctinyintmin  TINYINT,
    ctinyintmax  TINYINT,
    ctinyuint    TINYUINT,
    csmallintmin SMALLINT,
    csmallintmax SMALLINT,
    csmalluint   SMALLUINT,
    cintmin      INT,
    cintmax      INT,
    cuint        UINT,
    cbigintmin   BIGINT,
    cbigintmax   BIGINT,
    cbiguint     BIGUINT,
    cfloatmin    FLOAT,
    cfloatmax    FLOAT,
    cdoublemin   DOUBLE,
    cdoublemax   DOUBLE,
    ctext        TEXT,
    cts          TIMESTAMP NOT NULL
);

INSERT INTO testdb.table_with_all_data_types VALUES (
    -128,
    127,
    255,
    -32768,
    32767,
    65535,
    -2147483648,
    2147483647,
    4294967295,
    -9223372036854775808,
    9223372036854775807,
    18446744073709551615,
    222.222,
    222.222,
    222.222,
    222.222,
    '汉字',
    CURRENT_TIMESTAMP
);
