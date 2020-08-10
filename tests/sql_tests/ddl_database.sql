-- Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
-- Use of this source code is governed by a license that can be found
-- in the LICENSE file.

use database sys;
create database db_default;
use database db_default;
use database sys;
drop database db_default;

create database db_aes128 with cipher_id = 'aes128',
cipher_key_seed = 'myencryptionkey128';
use database db_aes128;
use database sys;
drop database db_aes128;

create database db_aes256 with cipher_id = 'aes256',
cipher_key_seed = 'myencryptionkey256';
use database db_aes256;
use database sys;
drop database db_aes256;

create database db_camellia128 with cipher_id = 'camellia128',
cipher_key_seed = 'myencryptionkeycamellia128';
use database db_camellia128;
use database sys;
drop database db_camellia128;

create database db_none with cipher_id = 'none';
use database db_none;
use database sys;
drop database db_none;
