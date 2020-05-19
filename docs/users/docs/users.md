# User Management

## List existing users

```sql
siocli> select name, state from sys_users ;

NAME                 STATE
-------------------- -----
                root     1

1 rows.
```

## Create a new user with its key

```sql
siocli> create user nico ;
Command execution time: 18 ms.

siocli> alter user nico add access key main 'ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDBOzgmO6E8xJAWz0CyzG8/FWJ+0oTTbPqX1c0JEKufxyHdS8VyTl6BuL7aIYt5RiUc+V1bzOKt0guPCu8WKIgeb1nq3qtvWaswJBod6iWs6iN1y+6+/oT47CrgWZUi9LLseGxit8DQHeCshTvaB8e6ZFH2sZdTpS8Z7U86znNnfX/7qoXUqEXVLawBKC8NGgWpvjvi0ZK9AF8ckD9p4Tdcoy+8m3+aFitbv1i7mVY+hJ7pDlRp6YeJYKC3kC46Bp41G2x0tpgls0HzpIEMBedV95aVxECrPxcAEkooIMWCJBbTEP7mc6Sb0H82p1QE2zrluW/L/S82NCLLWqsqm5An nico@siodb' ;
Command execution time: 21 ms.
```

## Additional notes

- By default user with active state can login to Siodb.
- To generate a key, you can use [`ssh-keygen`](https://www.ssh.com/ssh/keygen).
- You can add as much key as you need for a user.