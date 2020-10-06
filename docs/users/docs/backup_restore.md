# Backup and restore

## Backup

You must backup your database on a regular basis. A restore from backup is the last resort when
everything is lost. There are two types of backup you can do with Siodb: logical backup and
physical backup. The former is an export from the data and structure in a form of SQL ready to
be a replay in a fresh environment. The later is a copy of the data files of Siodb.

## Logical backup (export)

### Export one database

```bash
siocli --user root --export <database_name>
```

### Export all databases of an instance

```bash
siocli --user root --export-all
```

### Pysical backup

1. Stop the database

```bash
TODO
```

2. Copy the datafiles to the backup location

```bash
TODO
```

3. Start the database

```bash
TODO
```

## Restore


### Restore of one database

```bash
TODO
```

### Restore all databases of an instance

```bash
TODO
```

### Restore from pysical backup

1. Stop the database

```bash
TODO
```

2. Copy the datafiles from the backup location to Siodb back to Siodb data directory

```bash
TODO
```

3. Start the database

```bash
TODO
```
