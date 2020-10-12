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

## Create your first database

```sql
create database myapp ; -- Creates an encrypted database with the default cipher AES128
```

### Create your first table

```sql
use database myapp ;

create table employees ( firstname text, lastname text, salary float, hire_date timestamp) ;

insert into employees ( firstname, lastname, salary, hire_date)
values
( '马', '云', 249000.00, '1964-09-10' ),
( 'Ю́рий', 'Алексе́евич Гага́рин', 49000.00, '1934-03-09' ),
( 'Barack', 'Obama', 149000.00, '1961-08-04' )
;
```

## REST API

### Add a token access to root user

```bash
TOKEN=$(siocli --user root <<< 'alter user root add token TOKEN1' | grep 'Server: token:' | awk '{print $3}')
```

### POST Request

```bash
curl -k -X POST \
-d '[
    { "firstname": "马","lastname": "云","salary": "249000.00","hire_date": "1964-09-10"},
    { "firstname": "Ю́рий","lastname": "Алексе́евич Гага́рин","salary": "49000.00","hire_date": "1934-03-09"},
    { "firstname": "Barack","lastname": "Obama","salary": "149000.00","hire_date": "1961-08-04"}
]' \
https://root:${TOKEN}@localhost:50443/databases/myapp/tables/employees/rows
```

**Returns:**

```json
{
	"status": 200,
	"affectedRowCount": 3,
	"trids": [1, 2, 3]
}
```

### GET Request

```bash
curl -s -k https://root:${TOKEN}@localhost:50443/databases/myapp/tables/employees/rows
```

**Returns:**

```json
{
   "rows" : [
      {
         "FIRSTNAME" : "马",
         "HIRE_DATE" : "1964-09-10",
         "LASTNAME" : "云",
         "SALARY" : 249000,
         "TRID" : 1
      },
      {
         "FIRSTNAME" : "Ю́рий",
         "HIRE_DATE" : "1934-03-09",
         "LASTNAME" : "Алексе́евич Гага́рин",
         "SALARY" : 49000,
         "TRID" : 2
      },
      {
         "FIRSTNAME" : "Barack",
         "HIRE_DATE" : "1961-08-04",
         "LASTNAME" : "Obama",
         "SALARY" : 149000,
         "TRID" : 3
      }
   ],
   "status" : 200
}
```
