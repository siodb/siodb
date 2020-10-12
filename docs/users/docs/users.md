# Users

## List existing users

```sql
select name, state from sys_users ;
```

## Create a user

```sql
create user <user_name> ;
```

### Create a user with attributes

```sql
create user <user_name>
with state = ACTIVE|INACTIVE, real_name = 'User Real Name',
description = 'Description for user' ;
```

**Note:** By default user are created with an active state. A state 'INACTIVE' means that
the user cannot login.

### Alter a user

```sql
alter user <user_name>
set state = ACTIVE|INACTIVE, real_name = 'User Real Name',
description = 'Description for user' ;
```

### Alter user authentication access

See [authentication page](authentication.md).

## Drop a new user

```sql
drop user <user_name> ;
```
