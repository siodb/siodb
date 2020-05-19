# System tables

This page reference the tables that Siodb manages to function.
Siodb adds an internal column `TRID`. Siodb uses that column as a primary key for all
rows in a table. Siodb increment the `TRID` at each `INSERT` operation.
The number is unsigned INT that can grow up to 18446744073709551615.

## sys_tables

- trid
- type
- name
- first_user_trid
- current_column_set_id

## sys_dummy

- trid
- dummy

## sys_column_sets

- trid
- column_count

## sys_columns

- trid
- data_type
- name
- state
- block_data_area_size

## sys_column_defs

- trid
- column_id
- constraint_count

## sys_column_set_columns

- trid
- column_set_id
- column_def_id

## sys_constraint_defs

- trid
- type
- expr

## sys_constraints

- trid
- name
- state
- column_id
- def_id

## sys_column_def_constraints

- trid
- column_def_id
- constraint_id

## sys_indices

- trid
- type
- unique
- name
- data_file_size

## sys_index_columns

- trid
- index_id
- column_def_id
- sort_desc

## sys_users

- trid
- name
- real_name
- state

## sys_user_access_keys

- trid
- user_id
- name
- text
- state

## sys_databases

- trid
- uuid
- name
- cipher_id
- cipher_key

## sys_user_permissions

- trid
- user_id
- database_id
- object_type
- object_id
- permissions
- grant_options

