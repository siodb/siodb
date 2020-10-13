
# Authentication

To access a Siodb instance, you need a user who has an authentication mechanism.
A user can be authenticated by two means:

- SQL access: key pair.
- REST access: Token.

**Note:** This page does not describe how to manage users. To learn how to manage user,
please go to the [User Administration page](users.md).

## SQL Access

The Siodb user must have a public access key added. The public key can be in one of
the following formats:

- RSA 2048 bit and higher
- DH 2048 bit and higher
- ECDH
- ED25519

### Add an public key

```sql
alter user <user_name> add access key <key_name> 'ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDBOzgmO6E8xJAWz0CyzG8/FWJ+0oTTbPqX1c0JEKufxyHdS8VyTl6BuL7aIYt5RiUc+V1bzOKt0guPCu8WKIgeb1nq3qtvWaswJBod6iWs6iN1y+6+/oT47CrgWZUi9LLseGxit8DQHeCshTvaB8e6ZFH2sZdTpS8Z7U86znNnfX/7qoXUqEXVLawBKC8NGgWpvjvi0ZK9AF8ckD9p4Tdcoy+8m3+aFitbv1i7mVY+hJ7pDlRp6YeJYKC3kC46Bp41G2x0tpgls0HzpIEMBedV95aVxECrPxcAEkooIMWCJBbTEP7mc6Sb0H82p1QE2zrluW/L/S82NCLLWqsqm5An' ;
```

It's also possible to create a user with attributes to a user public key:

```sql
alter user <user_name> add access key <key_name> 'ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDBOzgmO6E8xJAWz0CyzG8/FWJ+0oTTbPqX1c0JEKufxyHdS8VyTl6BuL7aIYt5RiUc+V1bzOKt0guPCu8WKIgeb1nq3qtvWaswJBod6iWs6iN1y+6+/oT47CrgWZUi9LLseGxit8DQHeCshTvaB8e6ZFH2sZdTpS8Z7U86znNnfX/7qoXUqEXVLawBKC8NGgWpvjvi0ZK9AF8ckD9p4Tdcoy+8m3+aFitbv1i7mVY+hJ7pDlRp6YeJYKC3kC46Bp41G2x0tpgls0HzpIEMBedV95aVxECrPxcAEkooIMWCJBbTEP7mc6Sb0H82p1QE2zrluW/L/S82NCLLWqsqm5An'
with state = ACTIVE|INACTIVE, description = 'A description for key 1' ;
```

**Note:**

- To generate a key, you can use [`ssh-keygen`](https://www.ssh.com/ssh/keygen).
- You can add as much key as you need for a user.

### Alter an access key

```sql
alter user <user_name> alter access key <key_name>
set state = ACTIVE|INACTIVE, description = 'A description for key 1';
```

### Drop an access key

```sql
alter user <user_name> drop access key <key_name> ;
```

## REST API Access

The authentication through REST is done with [HTTP Basic authentication](https://tools.ietf.org/html/rfc2617).
The username must be a user in the Siodb instance, and the password must be a token that belongs to this user.
The URL to the REST API can be:

`https://<USER_NAME>:<USER_TOKEN>@<SIODB_SERVER>:<SIODB_INSTANCE_PORT>/<PATH>`

### Add a token

You can generate a token for a user using SQL command:

```sql
alter user <user_name> add token <token_name>;
```

Generated tokens are guaranteed at least to be unique among all currently existing tokens for the
mentioned used. Generated token is returned in the `DatabaseEngineReponse.freetext_message` in the
following format: `token: xxxxxxxx...xx`, where `xx...` are hexadecimal digits. There must be an
even number of hexadecimal digits. The token is not stored in the database in this cleartext form,
and if the token is lost, there is no way to restore it. In such a case, you must generate a new token.

Token can be assigned expiration time:

```sql
alter user <user_name> add token <token_name>
with expiration_time='yyyy-mm-dd hh:mm:ss';
```

It is possible to add token with supplied value:

```sql
alter user <user_name> add token <token_name> x'xxxx...xx';
```

Supplied value must be unique among existing tokens for the designated user;
otherwise, Siodb will not create the token.

### Alter a token

It is possible to change token properties with the following SQL commands:

```sql
alter user <user_name> alter token <token_name>
set expiration_time='yyyy-mm-dd hh:mm:ss',
description = 'Token description' ;
```

### Drop a token

It is possible to delete a token with the following SQL commands:

```sql
alter user <user_name> drop token <token_name>;
```

### Example with curl

```bash
curl https://root:0929b8e2f86a9c6c379a96cd8005072a09caf40affdf9b56098858f3d7188a1ab22bfcb4287b30b25aa1e7e8291a5605dc9b135ba2414e2c470a8ed7705d74d3@localhost:50443/databases/sys/tables/sys_tables/rows`
```
