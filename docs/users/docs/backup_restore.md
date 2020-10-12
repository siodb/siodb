# Backup and restore

## Backup

You must backup your database on a regular basis and before any major database operation (upgrade,
etc). A restore from backup is the last resort when everything is lost. There are two types of
backup you can do with Siodb: logical backup and physical backup. The former is an export from
the data and structure in a form of SQL ready to be a replay in a fresh environment. The later is
a copy of the data files of Siodb.

**Warning:** it is a very good practice to verify that the backups you make are consistent and can be
restored at any time. A backup procedure not tested with a restore on a different system is the same
as not having a backup procedure.

## Logical backup (export)

### Export one database

```bash
siocli --user root --export <DATABASE NAME>
```

**Example:**

```bash
siocli --user root --export TEST_DB1
```

**Output:**

```sql
-- Siodb SQL Dump
-- Hostname: localhost
-- Instance:
-- Timestamp: 2020-10-12 09:14:59
-- Timestamp (UTC): 2020-10-12 09:14:59

-- Database: TEST_DB1
CREATE DATABASE TEST_DB1 WITH CIPHER_ID = 'aes128';

-- Table: TEST_DB1.EMPLOYEES
CREATE TABLE TEST_DB1.EMPLOYEES (FIRSTNAME TEXT, LASTNAME TEXT, SALARY FLOAT, HIRE_DATE TIMESTAMP);
INSERT INTO TEST_DB1.EMPLOYEES VALUES ('马', '云', 249000.000000, '1964-09-10');
INSERT INTO TEST_DB1.EMPLOYEES VALUES ('Ю́рий', 'Алексе́евич Гага́рин', 49000.000000, '1934-03-09');
INSERT INTO TEST_DB1.EMPLOYEES VALUES ('Barack', 'Obama', 149000.000000, '1961-08-04');
```


### Export all databases of an instance

```bash
siocli --user root --export-all
```

**Example:**

```bash
siocli --user root --export-all
```

**Output:**

```sql
-- Siodb SQL Dump
-- Hostname: localhost
-- Instance:
-- Timestamp: 2020-10-12 09:12:13
-- Timestamp (UTC): 2020-10-12 09:12:13

-- Database: TEST_DB1
CREATE DATABASE TEST_DB1 WITH CIPHER_ID = 'aes128';

-- Table: TEST_DB1.EMPLOYEES
CREATE TABLE TEST_DB1.EMPLOYEES (FIRSTNAME TEXT, LASTNAME TEXT, SALARY FLOAT, HIRE_DATE TIMESTAMP);
INSERT INTO TEST_DB1.EMPLOYEES VALUES ('马', '云', 249000.000000, '1964-09-10');
INSERT INTO TEST_DB1.EMPLOYEES VALUES ('Ю́рий', 'Алексе́евич Гага́рин', 49000.000000, '1934-03-09');
INSERT INTO TEST_DB1.EMPLOYEES VALUES ('Barack', 'Obama', 149000.000000, '1961-08-04');


-- Database: TEST_DB2
CREATE DATABASE TEST_DB2 WITH CIPHER_ID = 'aes256';

-- Table: TEST_DB2.EMPLOYEES
CREATE TABLE TEST_DB2.EMPLOYEES (FIRSTNAME TEXT, LASTNAME TEXT, SALARY FLOAT, HIRE_DATE TIMESTAMP);
INSERT INTO TEST_DB2.EMPLOYEES VALUES ('马', '云', 249000.000000, '1964-09-10');
INSERT INTO TEST_DB2.EMPLOYEES VALUES ('Ю́рий', 'Алексе́евич Гага́рин', 49000.000000, '1934-03-09');
INSERT INTO TEST_DB2.EMPLOYEES VALUES ('Barack', 'Obama', 149000.000000, '1961-08-04');


-- Database: TEST_DB3
CREATE DATABASE TEST_DB3 WITH CIPHER_ID = 'none';

-- Table: TEST_DB3.EMPLOYEES
CREATE TABLE TEST_DB3.EMPLOYEES (FIRSTNAME TEXT, LASTNAME TEXT, SALARY FLOAT, HIRE_DATE TIMESTAMP);
INSERT INTO TEST_DB3.EMPLOYEES VALUES ('马', '云', 249000.000000, '1964-09-10');
INSERT INTO TEST_DB3.EMPLOYEES VALUES ('Ю́рий', 'Алексе́евич Гага́рин', 49000.000000, '1934-03-09');
INSERT INTO TEST_DB3.EMPLOYEES VALUES ('Barack', 'Obama', 149000.000000, '1961-08-04');
```

### Pysical backup

1. Stop the siodb instance:

```bash
systemctl stop siodb<SIODB_VERSION>
```

2. Copy the datafiles to the backup location:

```bash
tar -zcvf <BACKUP ARCHIVE NAME>.tar.gz <INSTANCE DATA FILE DIRECTORY>
```

**Example:**

```bash
tar -pcvzf backup_siodb_instance_siodb000_`date +"%y-%m-%d_%H-%M-%s"`.tar.gz /var/lib/siodb/siodb000/data
```

3. Start the siodb instance:

```bash
systemctl stop siodb<SIODB_VERSION>
```

## Restore

### Restore from a SQL dump

To restore a database, you can send the content of a SQL dump file into the STDIN of the `siocli` process:

```bash
siocli --nologo --user root < <SQL DUMP FILE>
```

**Example:**

```bash
siocli --nologo --user root < /tmp/dump_SQL_db_TEST_DB4.sql
```

**Output:**

```bash
CREATE DATABASE TEST_DB4 WITH CIPHER_ID = 'aes128';

Connected to localhost:50000
Command execution time: 7892 ms.

CREATE TABLE TEST_DB4.EMPLOYEES (FIRSTNAME TEXT, LASTNAME TEXT, SALARY FLOAT, HIRE_DATE TIMESTAMP);

Command execution time: 717 ms.

INSERT INTO TEST_DB4.EMPLOYEES VALUES ('马', '云', 249000.000000, '1964-09-10');

1 rows affected
Command execution time: 4169 ms.

INSERT INTO TEST_DB4.EMPLOYEES VALUES ('Ю́рий', 'Алексе́евич Гага́рин', 49000.000000, '1934-03-09');

1 rows affected
Command execution time: 31 ms.

INSERT INTO TEST_DB4.EMPLOYEES VALUES ('Barack', 'Obama', 149000.000000, '1961-08-04');

1 rows affected
Command execution time: 34 ms.
```

### Restore from pysical backup

1. Stop the siodb instance:

```bash
systemctl stop siodb<SIODB_VERSION>
```

2. Restore the data file into the Siodb data directory:

**Warning:** This may destroy files existing the Siodb data directory. You can verify where the files
are going to be restored by listing them from the archive before uncompressing the tar file. To
verify file location in the tar file, use this command:

```bash
tar -ptvzf <BACKUP ARCHIVE NAME>.tar.gz
```

If the files are listed in the desired location, you can restore them with this command:

```bash
tar -pxvzf <BACKUP ARCHIVE NAME>.tar.gz
```

**Example:**

```bash
tar -pxvzf backup_siodb_instance_siodb000_20-10-12_09-29-1602494985.tar.gz
```

1. Start the siodb instance:

```bash
systemctl stop siodb<SIODB_VERSION>
```
