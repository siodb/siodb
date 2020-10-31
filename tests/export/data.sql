-- Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
-- Use of this source code is governed by a license that can be found
-- in the LICENSE file.

-- Create some data to export

-- DB1
CREATE DATABASE db1;
USE DATABASE db1;

CREATE TABLE t1_db1(a int, b text);
INSERT INTO t1_db1 VALUES(1, 'abc');
INSERT INTO t1_db1 VALUES(2, 'def');
INSERT INTO t1_db1 VALUES(3, NULL);
INSERT INTO t1_db1 VALUES(4, 'ghi');
INSERT INTO t1_db1 VALUES(5, 'jkl');

CREATE TABLE t2_db1(a int, b text, c real not null);
INSERT INTO t2_db1 VALUES(200, 'qwe', 0.2);
INSERT INTO t2_db1 VALUES(199, 'rty', 1.4);
INSERT INTO t2_db1 VALUES(198, NULL, -50505.5356);
INSERT INTO t2_db1 VALUES(197, 'uio', 8236.45);
INSERT INTO t2_db1 VALUES(196, NULL, -105.56);
INSERT INTO t2_db1 VALUES(195, 'ppp', 0.365236);

-- DB2

CREATE DATABASE db2;
USE DATABASE db2;

CREATE TABLE t1_db2(a int, b text);
INSERT INTO t1_db2 VALUES(10, 'xyz');
INSERT INTO t1_db2 VALUES(11, 'xyzz');
INSERT INTO t1_db2 VALUES(12, 'xyzzz');
INSERT INTO t1_db2 VALUES(13, 'xyzzzz');
INSERT INTO t1_db2 VALUES(14, NULL);

CREATE TABLE t2_db2(a int, b text, c text not null);
INSERT INTO t2_db2 VALUES(10, 'xyz', 'abc');
INSERT INTO t2_db2 VALUES(11, 'xyzz', 'xyz');
INSERT INTO t2_db2 VALUES(12, 'xyzzz', 'def');
INSERT INTO t2_db2 VALUES(13, 'xyzzzz', 'абвгд');
INSERT INTO t2_db2 VALUES(14, NULL, 'ё');
