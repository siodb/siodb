-- Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
-- Use of this source code is governed by a license that can be found
-- in the LICENSE file.

-- Creates object necessary for the REST protocol tests

CREATE USER user1;

CREATE DATABASE db1;
USE DATABASE db1;

CREATE TABLE t1_1(a INT, b TEXT);
INSERT INTO t1_1 VALUES(100, 'a');
INSERT INTO t1_1 VALUES(101, 'b');
INSERT INTO t1_1 VALUES(102, 'c');
INSERT INTO t1_1 VALUES(103, 'd');
INSERT INTO t1_1 VALUES(104, 'e');

CREATE TABLE t1_2(a INT, b TEXT, c INT, d TEXT);
INSERT INTO t1_2 VALUES(100, 'a');
INSERT INTO t1_2 VALUES(101, 'b', 1, 'x');
INSERT INTO t1_2 VALUES(102, 'c');
INSERT INTO t1_2 VALUES(103, 'd', 2, 'y');
INSERT INTO t1_2 VALUES(104, 'e');
INSERT INTO t1_2 VALUES(105, 'f', 3, 'z');

CREATE DATABASE db2;
USE DATABASE db2;
CREATE TABLE t2_1(a INT, b TEXT);

CREATE DATABASE db3;
