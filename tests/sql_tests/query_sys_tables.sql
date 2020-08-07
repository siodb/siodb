-- Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
-- Use of this source code is governed by a license that can be found
-- in the LICENSE file.

-- Test: Query all system tables.
-- Ensures that all required columns are present.

SELECT trid, type, name, first_user_trid, current_column_set_id, description
FROM sys.sys_tables;

SELECT trid, dummy
FROM sys.sys_dummy;

SELECT trid, column_count
FROM sys.sys_column_sets;

SELECT trid, data_type, name, state, block_data_area_size, description
FROM sys.sys_columns;

SELECT trid, column_id, constraint_count
FROM sys.sys_column_defs;

SELECT trid, column_set_id, column_def_id
FROM sys.sys_column_set_columns;

SELECT trid, type, expr
FROM sys.sys_constraint_defs;

SELECT trid, name, state, column_id, def_id, description
FROM sys.sys_constraints;

SELECT trid, column_def_id, constraint_id
FROM sys.sys_column_def_constraints;

SELECT trid, type, unique, name, data_file_size, description
FROM sys.sys_indices;

SELECT trid, index_id, column_def_id, sort_desc
FROM sys.sys_index_columns;

SELECT trid, name, real_name, state, description
FROM sys.sys_users;

SELECT trid, user_id, name, text, state, description
FROM sys.sys_user_access_keys;

SELECT trid, user_id, name, value, expiration_timestamp, description
FROM sys.sys_user_tokens;

SELECT trid, uuid, name, cipher_id, cipher_key, description
FROM sys.sys_databases;

SELECT trid, user_id, database_id, object_type, object_id, permissions, grant_options
FROM sys.sys_user_permissions;
