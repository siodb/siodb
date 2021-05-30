-- Copyright (C) 2021 Siodb GmbH. All rights reserved.
-- Use of this source code is governed by a license that can be found
-- in the LICENSE file.

CREATE DATABASE db1;

CREATE TABLE db1.t1
(
    cINT INT,
    cTEXT TEXT
);

INSERT INTO db1.t1 VALUES(1, 'a1');
INSERT INTO db1.t1 VALUES(2, 'b1');
INSERT INTO db1.t1 VALUES(3, 'c1');
INSERT INTO db1.t1 VALUES(4, 'd1');

CREATE TABLE db1.t2
(
    cINT INT,
    cTEXT TEXT
);

INSERT INTO db1.t2 VALUES(1, 'a2');
INSERT INTO db1.t2 VALUES(2, 'b2');
INSERT INTO db1.t2 VALUES(3, 'c2');
INSERT INTO db1.t2 VALUES(4, 'd2');
