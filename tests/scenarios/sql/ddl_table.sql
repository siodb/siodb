-- Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
-- Use of this source code is governed by a license that can be found
-- in the LICENSE file.

drop database if exists db_default;
create database db_default;

use database db_default;

show tables;

drop table if exists table_all_datatypes;
create table table_all_datatypes (
    col_integer integer,
    col_int int,
    col_uint uint,
    col_tinyint tinyint,
    col_tinyuint tinyuint,
    col_smallint smallint,
    col_smalluint smalluint,
    col_bigint bigint,
    col_biguint biguint,
    col_smallreal smallreal,
    col_real real,
    col_float float,
    col_double double,
    col_text text,
    col_char char,
    col_varchar varchar,
    col_blob blob,
    col_timestamp timestamp
);

desc table table_all_datatypes;
show tables;

create table table_1
(
    col_int int,
    col_text text
);

desc table table_1;
show tables;

insert into table_1
values(1, 'a');
insert into table_1
values(2, 'b');

select *
from table_1;

drop table table_1;

show tables;

create table table_2
(
    col_int int not null,
    col_text text not null default 'hello world!!!'
);

desc table table_2;
show tables;

drop table table_2;

show tables;

create table table_1
(
    col_text2 text,
    col_int2 real
);

describe table table_1;
show tables;

insert into table_1
values('x', 100.51);
insert into table_1
values('y', 200.45);

select *
from table_1;

drop table table_1;

show tables;


-- Select dictionary
select *
from sys_tables;
select *
from sys_columns;

use database sys;
drop database db_default;
