create database erp_camellia128
with cipher_id='camellia128', cipher_key_seed = 'myencryptionkeycamellia128' ;

use database erp_camellia128 ;

create table test1 (
name text,
age int,
subscription_date timestamp
) ;

insert into test1 ( name, age, subscription_date ) values
( 'john doe', 48, '2016-02-29 12:10:29.123456789' ),
( 'jerome c. wagner', 25, '2016-02-29 12:10:29.123456789' ),
( 'nancy r. allen', 37, '2016-02-29 12:10:29.123456789' )
;

select *
from test1
where name in ( 'john doe', 'jerome c. wagner', 'nancy r. allen' ) ;

select *
from test1
where
name like '%om_ c. %'
or name like '_ancy r. a%'
and age = 25 ;

select alias.name full_name
from test1 alias
where alias.name like '%om_ c. %'
or alias.name like '_ancy r. a%' ;

update test1 set
age = 40
where name = 'john doe' ;

update test1 set
name = 'Debra C. Fenderson'
where subscription_date between '2015-01-01' and '2019-01-01' ;

update test1 set
subscription_date = '2009-01-03'
where name = 'nancy r. allen' ;

delete from test1
where name = 'jerome c. wagner' ;

delete from test1
where subscription_date between '2015-01-01' and '2019-01-01' ;

delete from test1 ;
delete from test1 where name = 'nancy r. allen' ;
delete from test1 where where trid = 9837498374 ;
delete from test1 where where name = 'nancy r. allen' and trid = 9837498374 ;
delete from test1 where where name = 'nancy r. allen' or trid = 9837498374 ;
update test1 set subscription_date = '2009-01-03' ;
update test1 set subscription_date = '2009-01-03' where name = 'nancy r. allen' ;
update test1 set subscription_date = '2009-01-03' where trid = 9837498374 ;
update test1 set subscription_date = '2009-01-03'
where name = 'nancy r. allen' and trid = 9837498374 ;
update test1 set subscription_date = '2009-01-03'
where name = 'nancy r. allen' or trid = 9837498374 ;

select sys_tables.name, sys_columns.name
from sys_tables, sys_columns
where sys_tables.trid = sys_columns.table_id ;

select sys_tables.trid, sys_tables.name, sys_columns.name
from sys_tables, sys_columns
where sys_tables.trid = sys_columns.table_id
and sys_tables.trid < 4096 ;

select tab.trid, tab.name, col.name, col.description
from sys_tables tab, sys_columns col
where tab.trid = col.table_id
and tab.trid < 4096 ;

select tab.trid, tab.name, col.name, col.description
from sys_tables tab, sys_columns col
where tab.trid = col.table_id
and tab.trid < 4096
limit 7 offset 3 ;

select tab.trid, tab.name, col.name, col.description
from sys_tables tab, sys_columns col
where tab.trid = col.table_id
and tab.trid < 4096
limit 3, 7 ;

create table table_all_datatypes (
test_integer integer,
test_int int,
test_uint uint,
test_tinyint tinyint,
test_tinyuint tinyuint,
test_smallint smallint,
test_smalluint smalluint,
test_bigint bigint,
test_biguint biguint,
test_smallreal smallreal,
test_real real,
test_float float,
test_double double,
test_text text,
test_char char,
test_varchar varchar,
test_blob blob,
test_timestamp timestamp) ;

insert into table_all_datatypes values (
382,
-182,
4294967295,
-127,
127,
-32767,
65535,
-4611686018427387904,
9223372036854775808,
45.3112,
45.3112,
45.3112,
4125.3112,
'az#@é)&☻❤♫☀")çue&"&éé篇题为',
'a',
'az#@é)&☻❤♫☀")çue&"&éé篇题为',
'0x73696f6462',
'2016-02-29 12:10:29.123456789'
), (
382,
-182,
4294967295,
-127,
127,
-32767,
65535,
-4611686018427387904,
9223372036854775808,
45.3112,
45.3112,
45.3112,
4125.3112,
'az#@é)&☻❤♫☀")çue&"&éé篇题为',
'a',
'az#@é)&☻❤♫☀")çue&"&éé篇题为',
'0x73696f6462',
'2016-02-29 12:10:29.123456789'
), (
382,
-182,
4294967295,
-127,
127,
-32767,
65535,
-4611686018427387904,
9223372036854775808,
45.3112,
45.3112,
45.3112,
4125.3112,
'az#@é)&☻❤♫☀")çue&"&éé篇题为',
'a',
'az#@é)&☻❤♫☀")çue&"&éé篇题为',
'0x73696f6462',
'2016-02-29 12:10:29.123456789'
), (
382,
-182,
4294967295,
-127,
127,
-32767,
65535,
-4611686018427387904,
9223372036854775808,
45.3112,
45.3112,
45.3112,
4125.3112,
'az#@é)&☻❤♫☀")çue&"&éé篇题为',
'a',
'az#@é)&☻❤♫☀")çue&"&éé篇题为',
'0x73696f6462',
'2016-02-29 12:10:29.123456789'
), (
382,
-182,
4294967295,
-127,
127,
-32767,
65535,
-4611686018427387904,
9223372036854775808,
45.3112,
45.3112,
45.3112,
4125.3112,
'az#@é)&☻❤♫☀")çue&"&éé篇题为',
'a',
'az#@é)&☻❤♫☀")çue&"&éé篇题为',
'0x73696f6462',
'2016-02-29 12:10:29.123456789'
);

update table_all_datatypes set test_integer   = 23                   where trid = 1 ;
update table_all_datatypes set test_int       = -69                  where trid = 2 ;
update table_all_datatypes set test_uint      = 7294967295           where trid = 3 ;
update table_all_datatypes set test_tinyint   = -98                  where trid = 4 ;
update table_all_datatypes set test_tinyuint  = 98                   where trid = 5 ;
update table_all_datatypes set test_smallint  = -36826               where trid = 1 ;
update table_all_datatypes set test_smalluint = 58293                where trid = 2 ;
update table_all_datatypes set test_bigint    = -3273879048427387904 where trid = 3 ;
update table_all_datatypes set test_biguint   = 8223372032233720308  where trid = 4 ;
update table_all_datatypes set test_smallreal = 85.9182              where trid = 5 ;
update table_all_datatypes set test_real      = 43.8273              where trid = 1 ;
update table_all_datatypes set test_float     = 76.3112              where trid = 2 ;
update table_all_datatypes set test_double    = 92735.9934           where trid = 3 ;
update table_all_datatypes set test_text      = '#@)&é)#@é☻❤&☻❤'   where trid = 4 ;
update table_all_datatypes set test_char      = 'b'                  where trid = 5 ;
update table_all_datatypes set test_varchar   = '☻❤&☻CSFQ34&é☻&☻'   where trid = 1 ;
update table_all_datatypes set test_blob      = '0x4c23b23a5d21f5'   where trid = 2 ;
update table_all_datatypes set test_timestamp = current_timestamp    where trid = 3 ;


use database sys ;

drop database erp_camellia128 ;

