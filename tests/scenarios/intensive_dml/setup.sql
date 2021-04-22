create database tdb
with cipher_id = 'camellia128',
cipher_key_seed = 'myencryptionkeycamellia128';

create table tdb.ttable (
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
) ;
