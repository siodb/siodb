# Quick Start

## Docker container

### Pull the last image of Siodb

```bash
docker pull siodb/siodb
docker run siodb/siodb
```

### Connect to the instance in the container

```sql
siodb@ea7ad13791c1:/$ siocli --host localhost --port 50000 --user root --identity-file ~/.ssh/id_rsa
Siodb client v.0.6.0
Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.

siocli> show databases ;

NAME                                     UUID
---------------------------------------- ----------------------------------------
                                     SYS     68ba038e-b704-2cb9-1d0d-b91864c819cd
```

## Installation on Linux

### Get the bootstrap script and run it

```bash
wget https://siodb.io/packages/InstallSiodb.sh
chmod u+x ./InstallSiodb.sh
sudo ./InstallSiodb.sh
```

### Connect to the instance

- Connect to your server as `siodb`

```bash
sudo su - siodb
siocli --host localhost --port 50000 --user root --identity-file ~/.ssh/id_rsa
```

- or (simplified)

```bash
sudo su - siodb
siocli --user root
```

### Create your first database

```sql
-- Create an encrypted database with the default cipher AES128
create database myapp ;
```

### Create your first table

```sql
use database myapp ;

create table employees ( firstname text, lastname text, salary float, hire_date timestamp) ;

insert into employees ( firstname, lastname, salary, hire_date)
values
( 'John', 'Doe', 180000, '2016-02-29' ) ;
```
