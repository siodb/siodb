-- Copyright (C) 2021 Siodb GmbH. All rights reserved.
-- Use of this source code is governed by a license that can be found
-- in the LICENSE file.

CREATE TABLE db1.t1
(
    cTEXT TEXT
);

INSERT INTO db1.t1
VALUES
    ('a1'),
    ('b1'),
    ('c1'),
    ('d1');

CREATE TABLE db1.t2
(
    cTEXT TEXT
);

INSERT INTO db1.t2
VALUES
    ('a2'),
    ('b2'),
    ('c2'),
    ('d2');

CREATE TABLE db1.t3
(
    cTEXT TEXT
);

INSERT INTO db1.t3
VALUES
    ('a2'),
    ('b2'),
    ('c2'),
    ('d2');
