-- Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
-- Use of this source code is governed by a license that can be found
-- in the LICENSE file.

create database db_aes256 with cipher_id = 'aes256',
cipher_key_seed = 'myencryptionkey256';
use database db_aes256;

create table table_timestamp (
start_date timestamp,
finish_date timestamp
);
insert into table_timestamp (start_date, finish_date) values (current_timestamp, current_timestamp);
insert into table_timestamp (start_date, finish_date) values ('2012-03-12 01:06:27', '2012-03-12');
insert into table_timestamp (start_date, finish_date) values ('9999-12-13 23:59:59', '9999-12-12');
insert into table_timestamp (start_date, finish_date) values ('2000-12-14 23:59:59.999999999', '1543-01-31');
insert into table_timestamp (start_date, finish_date) values ('2000-01-01 00:00:00', '2000-01-01');
insert into table_timestamp (start_date, finish_date) values ('2012-02-29 12:12:12.123456789', '2012-03-12');
insert into table_timestamp (start_date, finish_date) values ('2012-02-29 12:12:12', '2012-02-29');
insert into table_timestamp (start_date, finish_date) values ('1999-01-01 00:10:01', '2000-02-21');
insert into table_timestamp (start_date, finish_date) values ('1991-10-13 23:06:27', '2012-11-12');
insert into table_timestamp (start_date, finish_date) values ('2019-01-31 00:00:31', '2019-01-31');
insert into table_timestamp (start_date, finish_date) values ('2019-02-28 00:00:41', '2019-02-28');
insert into table_timestamp (start_date, finish_date) values ('2019-03-31 00:00:51', '2019-03-31');
insert into table_timestamp (start_date, finish_date) values ('2019-04-30 00:00:21', '2019-04-30');
insert into table_timestamp (start_date, finish_date) values ('2019-05-31 00:10:01', '2019-05-31');
insert into table_timestamp (start_date, finish_date) values ('2019-06-30 00:20:01', '2019-06-30');
insert into table_timestamp (start_date, finish_date) values ('2019-07-31 00:30:01', '2019-07-31');
insert into table_timestamp (start_date, finish_date) values ('2019-08-31 00:30:01', '2019-08-31');
insert into table_timestamp (start_date, finish_date) values ('2019-09-30 00:30:01', '2019-09-30');
insert into table_timestamp (start_date, finish_date) values ('2019-10-31 00:30:01', '2019-10-31');
insert into table_timestamp (start_date, finish_date) values ('2019-11-30 10:30:01', '2019-11-30');
insert into table_timestamp (start_date, finish_date) values ('2019-12-31 20:30:01', '2019-12-31');
insert into table_timestamp (start_date, finish_date) values ( current_timestamp, '1400-01-01');
insert into table_timestamp (start_date, finish_date) values ('2012-03-12 01:06:27', current_timestamp);
select * from table_timestamp;

use database sys;
drop database db_aes256;
