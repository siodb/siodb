/*
 * Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
 * Use of this source code is governed by a license that can be found
 * in the LICENSE file.
 *
 * This work is based on the following work:
 *
 * Project : sqlite-parser; an ANTLR4 grammar for SQLite https://github.com/bkiers/sqlite-parser
 * Developed by : Bart Kiers, bart@big-o.nl
 *
 * Licensed under the following terms:
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 by Bart Kiers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 * NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

grammar Siodb;

parse: ( sql_stmt_list | error)* EOF;

error:
	UNEXPECTED_CHAR {
       throw antlr4::RuntimeException("UNEXPECTED_CHAR=" + $UNEXPECTED_CHAR.text);
   };

sql_stmt_list: ';'* sql_stmt ( ';'+ sql_stmt)* ';'*;

sql_stmt: (K_EXPLAIN ( K_QUERY K_PLAN)?)? (
		alter_constraint_stmt
		| alter_database_stmt
		| alter_index_stmt
		| alter_table_stmt
		| alter_user_stmt
		| analyze_stmt
		| attach_stmt
		| begin_stmt
		| check_user_token_stmt
		| commit_stmt
		| compound_select_stmt
		| create_database_stmt
		| create_index_stmt
		| create_table_stmt
		| create_trigger_stmt
		| create_user_stmt
		| create_view_stmt
		| create_virtual_table_stmt
		| delete_stmt
		| delete_stmt_limited
		| describe_table_stmt
		| detach_stmt
		| drop_database_stmt
		| drop_index_stmt
		| drop_table_stmt
		| drop_trigger_stmt
		| drop_user_stmt
		| drop_view_stmt
		| factored_select_stmt
		| grant_database_permissions_stmt
		| grant_instance_permissions_stmt
		| grant_table_permissions_stmt
		| grant_view_permissions_stmt
		| grant_index_permissions_stmt
		| grant_trigger_permissions_stmt
		| insert_stmt
		| reindex_stmt
		| release_stmt
		| revoke_database_permissions_stmt
		| revoke_instance_permissions_stmt
		| revoke_table_permissions_stmt
		| revoke_view_permissions_stmt
		| revoke_index_permissions_stmt
		| revoke_trigger_permissions_stmt
		| rollback_stmt
		| savepoint_stmt
		| simple_select_stmt
		| select_stmt
		| show_databases_stmt
		| show_tables_stmt
		| update_stmt
		| update_stmt_limited
		| use_database_stmt1
		| use_database_stmt2
		| vacuum_stmt
	);

alter_database_stmt:
	K_ALTER K_DATABASE database_name (
		K_RENAME (K_IF K_EXISTS)? K_TO new_database_name
		| K_SET database_attr_list
	);

database_attr_list: database_attr (',' database_attr)*;

database_attr: K_DESCRIPTION '=' (STRING_LITERAL | K_NULL);

alter_constraint_stmt:
	K_ALTER K_CONSTRAINT constraint_spec (
		K_RENAME (K_IF K_EXISTS)? K_TO new_constraint_name
		| K_SET constraint_attr_list
	);

constraint_attr_list: constraint_attr (',' constraint_attr)*;

constraint_attr: K_DESCRIPTION '=' (STRING_LITERAL | K_NULL);

alter_index_stmt:
	K_ALTER K_INDEX index_spec (
		K_RENAME (K_IF K_EXISTS)? K_TO new_index_name
		| K_SET index_attr_list
	);

index_attr_list: index_attr (',' index_attr)*;

index_attr: K_DESCRIPTION '=' (STRING_LITERAL | K_NULL);

alter_table_stmt:
	K_ALTER K_TABLE table_spec (
		K_RENAME (K_IF K_EXISTS)? K_TO new_table_name
		| K_ADD K_COLUMN? column_def
		| K_DROP K_COLUMN? (K_IF K_EXISTS)? column_name
		| K_ALTER K_COLUMN (
			column_name K_RENAME (K_IF K_EXISTS)? K_TO new_column_name
			| column_def
		)
		| K_SET table_attr_list
	);

alter_user_stmt:
	K_ALTER K_USER user_name (
		| K_SET user_attr_list
		| K_ADD K_ACCESS K_KEY user_access_key_name user_access_key_text (
			K_WITH user_access_key_attr_list
		)?
		| K_DROP K_ACCESS K_KEY (K_IF K_EXISTS)? user_access_key_name
		| K_ALTER K_ACCESS K_KEY user_access_key_name K_RENAME (
			K_IF K_EXISTS
		)? K_TO new_user_access_key_name
		| K_ALTER K_ACCESS K_KEY user_access_key_name K_SET user_access_key_attr_list
		| K_ADD K_TOKEN user_token_name (user_token_value)? (
			K_WITH user_token_attr_list
		)?
		| K_DROP K_TOKEN (K_IF K_EXISTS)? user_token_name
		| K_ALTER K_TOKEN user_token_name K_RENAME (
			K_IF K_EXISTS
		)? K_TO new_user_token_name
		| K_ALTER K_TOKEN user_token_name K_SET user_token_attr_list
	);

analyze_stmt: K_ANALYZE ( database_name | table_or_index_spec)?;

attach_stmt: K_ATTACH K_DATABASE expr K_AS database_name;

begin_stmt:
	K_BEGIN (K_DEFERRED | K_IMMEDIATE | K_EXCLUSIVE)? (
		K_TRANSACTION transaction_name?
	)?;

check_user_token_stmt:
	K_CHECK K_TOKEN user_name '.' user_token_name user_token_value;

commit_stmt: (K_COMMIT | K_END) (K_TRANSACTION transaction_name?)?;

compound_select_stmt: (
		K_WITH K_RECURSIVE? common_table_expression (
			',' common_table_expression
		)*
	)? select_core (
		(K_UNION K_ALL? | K_INTERSECT | K_EXCEPT) select_core
	)+ (K_ORDER K_BY ordering_term ( ',' ordering_term)*)? (
		K_LIMIT simple_expr (( K_OFFSET | ',') simple_expr)?
	)?;

create_database_attr:
	K_CIPHER_ID '=' simple_expr
	| K_CIPHER_KEY_SEED '=' simple_expr
	| K_UUID '=' simple_expr
	| K_DATA_DIRECTORY_MUST_EXIST '=' simple_expr;

create_database_attr_list:
	create_database_attr (',' create_database_attr)*;

create_database_stmt:
	K_CREATE (K_TEMP | K_TEMPORARY)? K_DATABASE database_name (
		K_WITH create_database_attr_list
	)?;

create_index_stmt:
	K_CREATE K_UNIQUE? K_INDEX (K_IF K_NOT K_EXISTS)? (
		database_name '.'
	)? index_name K_ON table_name '(' indexed_column (
		',' indexed_column
	)* ')' (K_WHERE expr)?;

create_table_stmt:
	K_CREATE (K_TEMP | K_TEMPORARY)? K_TABLE (
		K_IF K_NOT K_EXISTS
	)? table_spec (
		'(' column_def (',' column_def)* (',' table_constraint)* ')' (
			K_WITHOUT IDENTIFIER
		)?
		| K_AS select_stmt
	);

create_trigger_stmt:
	K_CREATE (K_TEMP | K_TEMPORARY)? K_TRIGGER (
		K_IF K_NOT K_EXISTS
	)? (database_name '.')? trigger_name (
		K_BEFORE
		| K_AFTER
		| K_INSTEAD K_OF
	)? (
		K_DELETE
		| K_INSERT
		| K_UPDATE ( K_OF column_name ( ',' column_name)*)?
	) K_ON table_spec (K_FOR K_EACH K_ROW)? (K_WHEN expr)? K_BEGIN (
		(update_stmt | insert_stmt | delete_stmt | select_stmt) ';'
	)+ K_END;

create_user_stmt:
	K_CREATE K_USER user_name (K_WITH user_attr_list)?;

create_view_stmt:
	K_CREATE (K_TEMP | K_TEMPORARY)? K_VIEW (K_IF K_NOT K_EXISTS)? (
		database_name '.'
	)? view_name K_AS select_stmt;

create_virtual_table_stmt:
	K_CREATE K_VIRTUAL K_TABLE (K_IF K_NOT K_EXISTS)? (
		database_name '.'
	)? table_name K_USING module_name (
		'(' module_argument (',' module_argument)* ')'
	)?;

delete_stmt:
	with_clause? K_DELETE K_FROM aliased_qualified_table_name (
		K_WHERE expr
	)?;

delete_stmt_limited:
	with_clause? K_DELETE K_FROM aliased_qualified_table_name (
		K_WHERE expr
	)? (
		(K_ORDER K_BY ordering_term ( ',' ordering_term)*)? K_LIMIT expr (
			( K_OFFSET | ',') expr
		)?
	)?;

describe_table_stmt: (K_DESCRIBE | K_DESC) K_TABLE table_spec;

detach_stmt: K_DETACH K_DATABASE database_name;

drop_database_stmt:
	K_DROP K_DATABASE (K_IF K_EXISTS)? database_name;

drop_index_stmt:
	K_DROP K_INDEX (K_IF K_EXISTS)? (database_name '.')? index_name;

drop_table_stmt: K_DROP K_TABLE (K_IF K_EXISTS)? table_spec;

drop_trigger_stmt:
	K_DROP K_TRIGGER (K_IF K_EXISTS)? (database_name '.')? trigger_name;

drop_user_stmt: K_DROP K_USER user_name;

drop_view_stmt:
	K_DROP K_VIEW (K_IF K_EXISTS)? (database_name '.')? view_name;

///////////////////////////////////////////////////////////

grant_table_permissions_stmt:
	K_GRANT table_permission_list K_ON (K_TABLE)? table_spec_ex K_TO user_name grant_option_spec?;

revoke_table_permissions_stmt:
	K_REVOKE table_permission_list K_ON (K_TABLE)? table_spec_ex K_FROM user_name;

table_permission_list: table_permission (',' table_permission)*;

table_permission:
	K_ALL
	| K_READ_ONLY // alias to {K_SELECT, K_SHOW}
	| K_READ_WRITE // alias to {K_SELECT, K_SHOW, K_INSERT, K_UPDATE, K_DELETE}
	| K_SELECT
	| K_INSERT
	| K_UPDATE
	| K_DELETE
	| K_DROP
	| K_ALTER
	| K_SHOW;

grant_option_spec: K_WITH K_GRANT K_OPTION;

///////////////////////////////////////////////////////////

grant_view_permissions_stmt:
	K_GRANT view_permission_list K_ON K_VIEW view_spec_ex K_TO user_name grant_option_spec?;

revoke_view_permissions_stmt:
	K_REVOKE view_permission_list K_ON K_VIEW view_spec_ex K_FROM user_name;

view_permission_list: view_permission (',' view_permission)*;

view_permission:
	K_ALL
	| K_SELECT
	| K_READ_ONLY // alias to {K_SELECT, K_SHOW}
	| K_DROP
	| K_ALTER
	| K_SHOW;

///////////////////////////////////////////////////////////

grant_index_permissions_stmt:
	K_GRANT index_permission_list K_ON K_INDEX index_spec_ex K_TO user_name grant_option_spec?;

revoke_index_permissions_stmt:
	K_REVOKE index_permission_list K_ON K_INDEX index_spec_ex K_FROM user_name;

index_permission_list: index_permission (',' index_permission)*;

index_permission: K_ALL | K_DROP | K_ALTER | K_SHOW;

///////////////////////////////////////////////////////////

grant_trigger_permissions_stmt:
	K_GRANT trigger_permission_list K_ON K_TRIGGER trigger_spec_ex K_TO user_name grant_option_spec?
		;

revoke_trigger_permissions_stmt:
	K_REVOKE trigger_permission_list K_ON K_TRIGGER trigger_spec_ex K_FROM user_name;

trigger_permission_list:
	trigger_permission (',' trigger_permission)*;

trigger_permission:
	K_ALL
	| K_ENABLE
	| K_DISABLE
	| K_DROP
	| K_ALTER
	| K_SHOW;

///////////////////////////////////////////////////////////

grant_database_permissions_stmt:
	K_GRANT database_permission_list K_ON K_DATABASE database_name_ex K_TO user_name
		grant_option_spec?;

revoke_database_permissions_stmt:
	K_GRANT database_permission_list K_ON K_DATABASE database_name_ex K_FROM user_name;

database_permission_list:
	database_permission (',' database_permission)*;

database_permission:
	K_ALL
	// table permissions
	| (K_CREATE K_TABLE)
	| (K_DROP K_ANY K_TABLE)
	| (K_ALTER K_ANY K_TABLE)
	| (K_SHOW K_ANY K_TABLE)
	// view permission
	| (K_CREATE K_VIEW)
	| (K_DROP K_ANY K_VIEW)
	| (K_ALTER K_ANY K_VIEW)
	| (K_SHOW K_ANY K_VIEW)
	// index permission
	| (K_CREATE K_INDEX)
	| (K_DROP K_ANY K_INDEX)
	| (K_ALTER K_ANY K_INDEX)
	| (K_SHOW K_ANY K_INDEX)
	// trigger permissions
	| (K_CREATE K_TRIGGER)
	| (K_DROP K_ANY K_TRIGGER)
	| (K_ALTER K_ANY K_TRIGGER)
	| (K_SHOW K_ANY K_TRIGGER)
	| K_DROP
	| K_ALTER
	| K_DETACH;

///////////////////////////////////////////////////////////

grant_instance_permissions_stmt:
	K_GRANT instance_permission_list K_TO user_name grant_option_spec?;

revoke_instance_permissions_stmt:
	K_REVOKE instance_permission_list K_FROM user_name;

instance_permission_list:
	instance_permission (',' instance_permission)*;

instance_permission:
	(K_CREATE K_DATABASE)
	| (K_DROP K_ANY K_DATABASE)
	| (K_ALTER K_ANY K_DATABASE)
	| (K_ATTACH K_DATABASE)
	| (K_DETACH K_ANY K_DATABASE)
	| (K_CREATE K_USER)
	| (K_ALTER K_USER)
	| (K_DROP K_USER)
	| (K_CREATE K_USER K_ACCESS K_KEY)
	| (K_ALTER K_USER K_ACCESS K_KEY)
	| (K_DROP K_USER K_ACCESS K_KEY)
	| (K_CREATE K_USER K_ACCESS K_TOKEN)
	| (K_ALTER K_USER K_ACCESS K_TOKEN)
	| (K_DROP K_USER K_ACCESS K_TOKEN);

///////////////////////////////////////////////////////////

factored_select_stmt: (
		K_WITH K_RECURSIVE? common_table_expression (
			',' common_table_expression
		)*
	)? select_core (compound_operator select_core)* (
		K_ORDER K_BY ordering_term (',' ordering_term)*
	)? (K_LIMIT simple_expr ( ( K_OFFSET | ',') simple_expr)?)?;

insert_stmt:
	with_clause? (
		K_INSERT
		| K_REPLACE
		| K_INSERT K_OR K_REPLACE
		| K_INSERT K_OR K_ROLLBACK
		| K_INSERT K_OR K_ABORT
		| K_INSERT K_OR K_FAIL
		| K_INSERT K_OR K_IGNORE
	) K_INTO (database_name '.')? table_name (
		'(' column_name (',' column_name)* ')'
	)? (
		K_VALUES '(' expr (',' expr)* ')' (
			',' '(' expr ( ',' expr)* ')'
		)*
		| select_stmt
		| K_DEFAULT K_VALUES
	);

table_attr: K_NEXT_TRID '=' NUMERIC_LITERAL;

table_attr_list: table_attr (',' table_attr)*;

reindex_stmt:
	K_REINDEX (
		collation_name
		| ( database_name '.')? ( table_name | index_name)
	)?;

release_stmt: K_RELEASE K_SAVEPOINT? savepoint_name;

rollback_stmt:
	K_ROLLBACK (K_TRANSACTION transaction_name?)? (
		K_TO K_SAVEPOINT? savepoint_name
	)?;

savepoint_stmt: K_SAVEPOINT savepoint_name;

simple_select_stmt: (
		K_WITH K_RECURSIVE? common_table_expression (
			',' common_table_expression
		)*
	)? select_core (
		K_ORDER K_BY ordering_term (',' ordering_term)*
	)? (K_LIMIT simple_expr ( ( K_OFFSET | ',') simple_expr)?)?;

select_stmt: (
		K_WITH K_RECURSIVE? common_table_expression (
			',' common_table_expression
		)*
	)? select_or_values (compound_operator select_or_values)* (
		K_ORDER K_BY ordering_term (',' ordering_term)*
	)? (K_LIMIT simple_expr ( ( K_OFFSET | ',') simple_expr)?)?;

select_or_values:
	K_SELECT (K_DISTINCT | K_ALL)? result_column (
		',' result_column
	)* (
		K_FROM (
			table_or_subquery (',' table_or_subquery)*
			| join_clause
		)
	)? (K_WHERE expr)? (
		K_GROUP K_BY expr (',' expr)* (K_HAVING expr)?
	)?
	| K_VALUES '(' expr (',' expr)* ')' (
		',' '(' expr ( ',' expr)* ')'
	)*;

show_databases_stmt: K_SHOW ( K_DATABASES | K_DBS);

show_tables_stmt: K_SHOW K_TABLES;

update_stmt:
	with_clause? K_UPDATE (
		K_OR K_ROLLBACK
		| K_OR K_ABORT
		| K_OR K_REPLACE
		| K_OR K_FAIL
		| K_OR K_IGNORE
	)? aliased_qualified_table_name K_SET column_name '=' expr (
		',' column_name '=' expr
	)* (K_WHERE expr)?;

update_stmt_limited:
	with_clause? K_UPDATE (
		K_OR K_ROLLBACK
		| K_OR K_ABORT
		| K_OR K_REPLACE
		| K_OR K_FAIL
		| K_OR K_IGNORE
	)? aliased_qualified_table_name K_SET column_name '=' expr (
		',' column_name '=' expr
	)* (K_WHERE expr)? (
		(K_ORDER K_BY ordering_term ( ',' ordering_term)*)? K_LIMIT expr (
			( K_OFFSET | ',') expr
		)?
	)?;

use_database_stmt1: K_USE database_name;
use_database_stmt2: K_USE K_DATABASE database_name;

vacuum_stmt: K_VACUUM;

column_def: column_name type_name? column_constraint*;

user_access_key_attr:
	K_STATE '=' (K_ACTIVE | K_INACTIVE)
	| K_DESCRIPTION '=' (STRING_LITERAL | K_NULL);

user_access_key_attr_list:
	user_access_key_attr (',' user_access_key_attr)*;

user_attr:
	K_STATE '=' (K_ACTIVE | K_INACTIVE)
	| K_REAL_NAME '=' (STRING_LITERAL | K_NULL)
	| K_DESCRIPTION '=' (STRING_LITERAL | K_NULL);

user_attr_list: user_attr (',' user_attr)*;

user_token_attr_list: user_token_attr (',' user_token_attr)*;

user_token_attr:
	K_EXPIRATION_TIMESTAMP '=' (STRING_LITERAL | K_NULL)
	| K_DESCRIPTION '=' (STRING_LITERAL | K_NULL);

type_name:
	IDENTIFIER+ (
		'(' signed_number ')'
		| '(' signed_number ',' signed_number ')'
	)?;

column_constraint: (K_CONSTRAINT constraint_name)? (
		K_PRIMARY K_KEY (K_ASC | K_DESC)? conflict_clause K_AUTOINCREMENT?
		| K_NOT? K_NULL conflict_clause
		| K_UNIQUE conflict_clause
		| K_CHECK '(' expr ')'
		| K_DEFAULT (
			signed_number
			| literal_value
			| '(' expr ')'
		)
		| K_COLLATE collation_name
		| foreign_key_clause
	);

conflict_clause: (
		K_ON K_CONFLICT (
			K_ROLLBACK
			| K_ABORT
			| K_FAIL
			| K_IGNORE
			| K_REPLACE
		)
	)?;

function_call:
	function_name '(' (K_DISTINCT? expr ( ',' expr)* | '*')? ')';

// Siodb understands the following binary operators, in order from highest to lowest precedence: ||
// * / % + - << >> & | ^ < <= > >= = == != <> IS IN LIKE GLOB MATCH REGEXP
simple_expr:
	literal_value
	| BIND_PARAMETER
	| ( ( database_name '.')? table_name '.')? column_name
	| unary_operator simple_expr
	| simple_expr '||' simple_expr
	| simple_expr ( '*' | '/' | '%') simple_expr
	| simple_expr ( '+' | '-') simple_expr
	| simple_expr ('<<' | '>>' | '&' | '|' | '^') simple_expr
	| simple_expr ( '<' | '<=' | '>' | '>=') simple_expr
	| simple_expr (
		'='
		| '=='
		| '!='
		| '<>'
		| K_IS
		| K_IS K_NOT
		| K_LIKE
		| K_GLOB
		| K_MATCH
		| K_REGEXP
	) simple_expr
	| '(' simple_expr ')'
	| K_CAST '(' simple_expr K_AS type_name ')'
	| simple_expr K_COLLATE collation_name
	| simple_expr K_NOT? (K_LIKE | K_GLOB | K_REGEXP | K_MATCH) simple_expr (
		K_ESCAPE simple_expr
	)?
	| simple_expr ( K_ISNULL | K_NOTNULL | K_NOT K_NULL)
	| simple_expr K_IS K_NOT? simple_expr
	| simple_expr K_NOT? K_BETWEEN simple_expr K_AND simple_expr
	| simple_expr K_NOT? K_IN (
		'(' (select_stmt | simple_expr ( ',' simple_expr)*)? ')'
		| ( database_name '.')? table_name
	)
	| ( ( K_NOT)? K_EXISTS)? '(' select_stmt ')'
	| K_CASE simple_expr? (K_WHEN simple_expr K_THEN simple_expr)+ (
		K_ELSE simple_expr
	)? K_END
	| raise_function;

expr:
	K_NOT expr
	| expr K_AND expr
	| expr K_OR expr
	| '(' expr ')'
	| simple_expr
	| function_call;

foreign_key_clause:
	K_REFERENCES foreign_table (
		'(' column_name (',' column_name)* ')'
	)? (
		(
			K_ON (K_DELETE | K_UPDATE) (
				K_SET K_NULL
				| K_SET K_DEFAULT
				| K_CASCADE
				| K_RESTRICT
				| K_NO K_ACTION
			)
			| K_MATCH name
		)
	)* (
		K_NOT? K_DEFERRABLE (
			K_INITIALLY K_DEFERRED
			| K_INITIALLY K_IMMEDIATE
		)?
	)?;

raise_function:
	K_RAISE '(' (
		K_IGNORE
		| ( K_ROLLBACK | K_ABORT | K_FAIL) ',' error_message
	) ')';

indexed_column:
	column_name (K_COLLATE collation_name)? (K_ASC | K_DESC)?;

table_constraint: (K_CONSTRAINT constraint_name)? (
		(K_PRIMARY K_KEY | K_UNIQUE) '(' indexed_column (
			',' indexed_column
		)* ')' conflict_clause
		| K_CHECK '(' expr ')'
		| K_FOREIGN K_KEY '(' column_name (',' column_name)* ')' foreign_key_clause
	);

with_clause:
	K_WITH K_RECURSIVE? cte_table_name K_AS '(' select_stmt ')' (
		',' cte_table_name K_AS '(' select_stmt ')'
	)*;

aliased_qualified_table_name: (database_name '.')? table_name (
		K_AS? table_alias
	)? (K_INDEXED K_BY index_name | K_NOT K_INDEXED)?;

ordering_term:
	expr (K_COLLATE collation_name)? (K_ASC | K_DESC)?;

common_table_expression:
	table_name ('(' column_name ( ',' column_name)* ')')? K_AS '(' select_stmt ')';

result_column:
	'*'
	| table_name '.' '*'
	| expr ( K_AS? column_alias)?;

table_or_subquery: (database_name '.')? table_name (
		K_AS? table_alias
	)? (K_INDEXED K_BY index_name | K_NOT K_INDEXED)?
	| '(' (
		table_or_subquery (',' table_or_subquery)*
		| join_clause
	) ')' (K_AS? table_alias)?
	| '(' select_stmt ')' ( K_AS? table_alias)?;

join_clause:
	table_or_subquery (
		join_operator table_or_subquery join_constraint
	)*;

join_operator:
	','
	| K_NATURAL? (K_LEFT K_OUTER? | K_INNER | K_CROSS)? K_JOIN;

join_constraint: (
		K_ON expr
		| K_USING '(' column_name ( ',' column_name)* ')'
	)?;

select_core:
	K_SELECT (K_DISTINCT | K_ALL)? result_column (
		',' result_column
	)* (
		K_FROM (
			table_or_subquery (',' table_or_subquery)*
			| join_clause
		)
	)? (K_WHERE expr)? (
		K_GROUP K_BY expr (',' expr)* (K_HAVING expr)?
	)?
	| K_VALUES '(' expr (',' expr)* ')' (
		',' '(' expr ( ',' expr)* ')'
	)*;

compound_operator:
	K_UNION
	| K_UNION K_ALL
	| K_INTERSECT
	| K_EXCEPT;

cte_table_name:
	table_name ('(' column_name ( ',' column_name)* ')')?;

signed_number: ( '+' | '-')? NUMERIC_LITERAL;

literal_value:
	NUMERIC_LITERAL
	| STRING_LITERAL
	| BLOB_LITERAL
	| K_NULL
	| K_CURRENT_TIME
	| K_CURRENT_DATE
	| K_CURRENT_TIMESTAMP
	| K_TRUE
	| K_FALSE;

unary_operator: '-' | '+' | '~';

error_message: STRING_LITERAL;

module_argument: expr | column_def;

column_alias: IDENTIFIER | STRING_LITERAL;

name: any_name;

function_name: any_name;

// TODO(siodb): ALTER FUNCTION RENAME IF EXISTS TO
new_function_name: any_name;

database_name: any_name;

database_name_ex: database_name | STAR;

// TODO(siodb): ALTER DATABASE RENAME IF EXISTS TO
new_database_name: any_name;

table_name: any_name;

table_spec: (database_name '.')? table_name;

table_spec_ex: (database_name '.')? (table_name | STAR);

table_or_index_name: any_name;

table_or_index_spec: (database_name '.')? table_or_index_name;

new_table_name: any_name;

column_name: any_name;

// TODO(siodb): ALTER TABLE ALTER COLUMN RENAME IF EXISTS TO
new_column_name: any_name;

constraint_name: any_name;

constraint_spec: (database_name '.')? constraint_name;

// TODO(siodb): ALTER TABLE ALTER CONSTRAINT RENAME IF EXISTS TO
new_constraint_name: any_name;

collation_name: any_name;

// TODO(siodb): ALTER COLLATION RENAME IF EXISTS TO
new_collation_name: any_name;

foreign_table: any_name;

index_name: any_name;

index_spec: (database_name '.')? index_name;

index_spec_ex: (database_name '.')? (index_name | STAR);

// TODO(siodb): ALTER INDEX RENAME IF EXISTS TO
new_index_name: any_name;

trigger_name: any_name;

trigger_spec: (database_name '.')? trigger_name;

trigger_spec_ex: (database_name '.')? (trigger_name | STAR);

// TODO(siodb): ALTER TRIGGER RENAME IF EXISTS TO
new_trigger_name: any_name;

user_name: any_name;

user_access_key_name: any_name;

user_access_key_text: STRING_LITERAL;

new_user_access_key_name: any_name;

user_token_name: any_name;

new_user_token_name: any_name;

user_token_value: BLOB_LITERAL;

view_name: any_name;

view_spec: (database_name '.')? view_name;

view_spec_ex: (database_name '.')? (view_name | STAR);

// TODO(siodb): ALTER VIEW RENAME IF EXISTS TO
new_view_name: any_name;

module_name: any_name;

savepoint_name: any_name;

table_alias: any_name;

transaction_name: any_name;

any_name:
	IDENTIFIER
	| attribute
	| keyword
	| STRING_LITERAL
	| '(' any_name ')';

SCOL: ';';
DOT: '.';
OPEN_PAR: '(';
CLOSE_PAR: ')';
COMMA: ',';
ASSIGN: '=';
STAR: '*';
PLUS: '+';
MINUS: '-';
TILDE: '~';
CARAT: '^';
PIPE2: '||';
DIV: '/';
MOD: '%';
LT2: '<<';
GT2: '>>';
AMP: '&';
PIPE: '|';
LT: '<';
LT_EQ: '<=';
GT: '>';
GT_EQ: '>=';
EQ: '==';
NOT_EQ1: '!=';
NOT_EQ2: '<>';

// Siodb specific: attributes
K_CIPHER_ID: C I P H E R '_' I D;
K_CIPHER_KEY_SEED: C I P H E R '_' K E Y '_' S E E D;
K_DESCRIPTION: D E S C R I P T I O N;
K_EXPIRATION_TIMESTAMP:
	E X P I R A T I O N '_' T I M E S T A M P;
K_NEXT_TRID: N E X T '_' T R I D;
K_STATE: S T A T E;
K_REAL_NAME: R E A L '_' N A M E;

// http://www.sqlite.org/lang_keywords.html
// + Siodb specific keywords
K_ABORT: A B O R T;
K_ACCESS: A C C E S S;
K_ACTION: A C T I O N;
K_ACTIVE: A C T I V E;
K_ADD: A D D;
K_AFTER: A F T E R;
K_ALL: A L L;
K_ALTER: A L T E R;
K_ANALYZE: A N A L Y Z E;
K_AND: A N D;
K_ANY: A N Y;
K_AS: A S;
K_ASC: A S C;
K_ATTACH: A T T A C H;
K_AUTOINCREMENT: A U T O I N C R E M E N T;
K_BEFORE: B E F O R E;
K_BEGIN: B E G I N;
K_BETWEEN: B E T W E E N;
K_BY: B Y;
K_CASCADE: C A S C A D E;
K_CASE: C A S E;
K_CAST: C A S T;
K_CHECK: C H E C K;
K_COLLATE: C O L L A T E;
K_COLUMN: C O L U M N;
K_COMMIT: C O M M I T;
K_CONFLICT: C O N F L I C T;
K_CONSTRAINT: C O N S T R A I N T;
K_CREATE: C R E A T E;
K_CROSS: C R O S S;
K_CURRENT_DATE: C U R R E N T '_' D A T E;
K_CURRENT_TIME: C U R R E N T '_' T I M E;
K_CURRENT_TIMESTAMP: C U R R E N T '_' T I M E S T A M P;
K_DATABASE: D A T A B A S E;
K_DATABASES: D A T A B A S E S;
K_DATA_DIRECTORY_MUST_EXIST:
	D A T A '_' D I R E C T O R Y '_' M U S T '_' E X I S T;
K_DBS: D B S;
K_DEFAULT: D E F A U L T;
K_DEFERRABLE: D E F E R R A B L E;
K_DEFERRED: D E F E R R E D;
K_DELETE: D E L E T E;
K_DESC: D E S C;
K_DESCRIBE: D E S C R I B E;
K_DETACH: D E T A C H;
K_DISABLE: D I S A B L E;
K_DISTINCT: D I S T I N C T;
K_DROP: D R O P;
K_EACH: E A C H;
K_ELSE: E L S E;
K_ENABLE: E N A B L E;
K_END: E N D;
K_ESCAPE: E S C A P E;
K_EXCEPT: E X C E P T;
K_EXCLUSIVE: E X C L U S I V E;
K_EXISTS: E X I S T S;
K_EXPLAIN: E X P L A I N;
K_FAIL: F A I L;
K_FALSE: F A L S E;
K_FOR: F O R;
K_FOREIGN: F O R E I G N;
K_FROM: F R O M;
K_FULL: F U L L;
K_GLOB: G L O B;
K_GRANT: G R A N T;
K_GROUP: G R O U P;
K_HAVING: H A V I N G;
K_IF: I F;
K_IGNORE: I G N O R E;
K_IMMEDIATE: I M M E D I A T E;
K_IN: I N;
K_INACTIVE: I N A C T I V E;
K_INDEX: I N D E X;
K_INDEXED: I N D E X E D;
K_INITIALLY: I N I T I A L L Y;
K_INNER: I N N E R;
K_INSERT: I N S E R T;
K_INSTEAD: I N S T E A D;
K_INTERSECT: I N T E R S E C T;
K_INTO: I N T O;
K_IS: I S;
K_ISNULL: I S N U L L;
K_JOIN: J O I N;
K_KEY: K E Y;
K_LEFT: L E F T;
K_LIKE: L I K E;
K_LIMIT: L I M I T;
K_MATCH: M A T C H;
K_NATURAL: N A T U R A L;
K_NO: N O;
K_NOT: N O T;
K_NOTNULL: N O T N U L L;
K_NULL: N U L L;
K_OF: O F;
K_OFFSET: O F F S E T;
K_ON: O N;
K_OPTION: O P T I O N;
K_OR: O R;
K_ORDER: O R D E R;
K_OUTER: O U T E R;
K_PLAN: P L A N;
K_PRIMARY: P R I M A R Y;
K_QUERY: Q U E R Y;
K_RAISE: R A I S E;
K_READ_ONLY: R E A D '_' O N L Y;
K_READ_WRITE: R E A D '_' W R I T E;
K_RECURSIVE: R E C U R S I V E;
K_REFERENCES: R E F E R E N C E S;
K_REGEXP: R E G E X P;
K_REINDEX: R E I N D E X;
K_RELEASE: R E L E A S E;
K_RENAME: R E N A M E;
K_REPLACE: R E P L A C E;
K_RESTRICT: R E S T R I C T;
K_REVOKE: R E V O K E;
K_RIGHT: R I G H T;
K_ROLLBACK: R O L L B A C K;
K_ROW: R O W;
K_SAVEPOINT: S A V E P O I N T;
K_SELECT: S E L E C T;
K_SET: S E T;
K_SHOW: S H O W;
K_TABLE: T A B L E;
K_TABLES: T A B L E S;
K_TEMP: T E M P;
K_TEMPORARY: T E M P O R A R Y;
K_THEN: T H E N;
K_TO: T O;
K_TOKEN: T O K E N;
K_TRANSACTION: T R A N S A C T I O N;
K_TRIGGER: T R I G G E R;
K_TRUE: T R U E;
K_UNION: U N I O N;
K_UNIQUE: U N I Q U E;
K_UPDATE: U P D A T E;
K_USE: U S E;
K_USER: U S E R;
K_USING: U S I N G;
K_UUID: U U I D;
K_VACUUM: V A C U U M;
K_VALUES: V A L U E S;
K_VIEW: V I E W;
K_VIRTUAL: V I R T U A L;
K_WHEN: W H E N;
K_WHERE: W H E R E;
K_WITH: W I T H;
K_WITHOUT: W I T H O U T;

keyword:
	K_ABORT
	| K_ACCESS
	| K_ACTION
	| K_ACTIVE
	| K_ADD
	| K_AFTER
	| K_ALL
	| K_ALTER
	| K_ANALYZE
	| K_AND
	| K_ANY
	| K_AS
	| K_ASC
	| K_ATTACH
	| K_AUTOINCREMENT
	| K_BEFORE
	| K_BEGIN
	| K_BETWEEN
	| K_BY
	| K_CASCADE
	| K_CASE
	| K_CAST
	| K_CHECK
	| K_COLLATE
	| K_COLUMN
	| K_COMMIT
	| K_CONFLICT
	| K_CONSTRAINT
	| K_CREATE
	| K_CROSS
	| K_CURRENT_DATE
	| K_CURRENT_TIME
	| K_CURRENT_TIMESTAMP
	| K_DATABASE
	| K_DATABASES
	| K_DBS
	| K_DEFAULT
	| K_DEFERRABLE
	| K_DEFERRED
	| K_DELETE
	| K_DESC
	| K_DESCRIBE
	| K_DETACH
	| K_DISABLE
	| K_DISTINCT
	| K_DROP
	| K_EACH
	| K_ELSE
	| K_ENABLE
	| K_END
	| K_ESCAPE
	| K_EXCEPT
	| K_EXCLUSIVE
	| K_EXISTS
	| K_EXPLAIN
	| K_FAIL
	| K_FALSE
	| K_FOR
	| K_FOREIGN
	| K_FROM
	| K_FULL
	| K_GLOB
	| K_GRANT
	| K_GROUP
	| K_HAVING
	| K_IF
	| K_IGNORE
	| K_IMMEDIATE
	| K_IN
	| K_INACTIVE
	| K_INDEX
	| K_INDEXED
	| K_INITIALLY
	| K_INNER
	| K_INSERT
	| K_INSTEAD
	| K_INTERSECT
	| K_INTO
	| K_IS
	| K_ISNULL
	| K_JOIN
	| K_KEY
	| K_LEFT
	| K_LIKE
	| K_LIMIT
	| K_MATCH
	| K_NATURAL
	| K_NO
	| K_NOT
	| K_NOTNULL
	| K_NULL
	| K_OF
	| K_OFFSET
	| K_ON
	| K_OPTION
	| K_OR
	| K_ORDER
	| K_OUTER
	| K_PLAN
	| K_PRIMARY
	| K_QUERY
	| K_RAISE
	| K_READ_ONLY
	| K_READ_WRITE
	| K_RECURSIVE
	| K_REFERENCES
	| K_REGEXP
	| K_REINDEX
	| K_RELEASE
	| K_RENAME
	| K_REPLACE
	| K_RESTRICT
	| K_REVOKE
	| K_RIGHT
	| K_ROLLBACK
	| K_ROW
	| K_SAVEPOINT
	| K_SELECT
	| K_SET
	| K_TABLE
	| K_TEMP
	| K_TEMPORARY
	| K_THEN
	| K_TO
	| K_TOKEN
	| K_TRANSACTION
	| K_TRIGGER
	| K_TRUE
	| K_UNION
	| K_UNIQUE
	| K_UPDATE
	| K_USE
	| K_USER
	| K_USING
	| K_VACUUM
	| K_VALUES
	| K_VIEW
	| K_VIRTUAL
	| K_WHEN
	| K_WHERE
	| K_WITH
	| K_WITHOUT;

attribute:
	K_CIPHER_ID
	| K_CIPHER_KEY_SEED
	| K_DATA_DIRECTORY_MUST_EXIST
	| K_EXPIRATION_TIMESTAMP
	| K_DESCRIPTION
	| K_NEXT_TRID
	| K_REAL_NAME
	| K_STATE
	| K_UUID;

IDENTIFIER:
	'"' (~'"' | '""')* '"'
	| '`' (~'`' | '``')* '`'
	| '[' ~']'* ']'
	| [a-zA-Z_] [a-zA-Z_0-9]*;

NUMERIC_LITERAL:
	DIGIT+ ('.' DIGIT*)? (E [-+]? DIGIT+)?
	| '.' DIGIT+ ( E [-+]? DIGIT+)?;

BIND_PARAMETER: '?' DIGIT* | [:@$] IDENTIFIER;

STRING_LITERAL: '\'' ( ~'\'' | '\'\'')* '\'';

BLOB_LITERAL: X STRING_LITERAL;

SINGLE_LINE_COMMENT: '--' ~[\r\n]* -> channel(HIDDEN);

MULTILINE_COMMENT: '/*' .*? ('*/' | EOF) -> channel(HIDDEN);

SPACES: [ \u000B\t\r\n] -> channel(HIDDEN);

UNEXPECTED_CHAR: .;

fragment DIGIT: [0-9];

fragment A: [aA];
fragment B: [bB];
fragment C: [cC];
fragment D: [dD];
fragment E: [eE];
fragment F: [fF];
fragment G: [gG];
fragment H: [hH];
fragment I: [iI];
fragment J: [jJ];
fragment K: [kK];
fragment L: [lL];
fragment M: [mM];
fragment N: [nN];
fragment O: [oO];
fragment P: [pP];
fragment Q: [qQ];
fragment R: [rR];
fragment S: [sS];
fragment T: [tT];
fragment U: [uU];
fragment V: [vV];
fragment W: [wW];
fragment X: [xX];
fragment Y: [yY];
fragment Z: [zZ];
