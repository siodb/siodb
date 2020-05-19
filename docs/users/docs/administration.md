# Administration

## Configuration file

Siodb identifies an instance by its name.
With the name of an instance, you can locate its configuration file.
Siodb expects to find the configuration file of an instance in the folder `/etc/siodb/instances/<INSTANCE_NAME>/`.
All files below must be present at each startup time:

- `/etc/siodb/instances/<INSTANCE_NAME>/config`: The Siodb parameter files.
- `/etc/siodb/instances/<INSTANCE_NAME>/system_db_key`: The master encryption
key for Siodb.

The file below must be present at the first startup time to indicate Siodb which
public key to use for the Siodb `root` user:

- `/etc/siodb/instances/<INSTANCE_NAME>/initial_access_key`

### Parameters

```bash
# Listening port for IPv4 client connections
# 0 means do not listen
ipv4_port = 50000

# Listening port for IPv6 client connections
# 0 means do not listen
ipv6_port = 0

# Data storage directory
data_dir = /var/lib/siodb/siodb/data

# Backlog value for the admin connection listener
admin_connection_listener_backlog = 10

# Max. number of admin connections
max_admin_connections = 10

# Backlog value for the user connection listener
user_connection_listener_backlog = 10

# Max. number of user connections
max_user_connections = 100

# IO Manager listening port for IPv4 client connections
# 0 means do not listen
iomgr.ipv4_port = 50001

# IO Manager listening port for IPv6 client connections
# 0 means do not listen
iomgr.ipv6_port = 0

# IO Manager worker thead number
iomgr.worker_thread_number = 2

# Database cache capacity
iomgr.database_cache_capacity = 100

# Table cache capacity
iomgr.table_cache_capacity = 100

# Capacity of the block cache (in 10M blocks)
iomgr.block_cache_capacity = 103

# Encryption default cipher id (aes128 is used if not set)
encryption.default_cipher_id = aes128

# Encryption algorithm used to encrypt system database (encryption.default_cipher_id is used if not set)
encryption.system_db_cipher_id = aes128

# Should encrypted connection be used for client (yes(default)/no)
client.enable_encryption = yes

# Client connection OpenSSL certificate
client.tls_certificate = /etc/siodb/instances/siodb/cert.pem

# Client connection OpenSSL certificate chain
# if both tls_certificate and tls_certificate_chain is set tls_certificate_chain is used
client.tls_certificate_chain = /etc/siodb/instances/siodb/certChain.pem

# Client secure connection certificate/certificate chain private key
client.tls_private_key = /etc/siodb/instances/siodb/key.pem

# Log channels
log_channels = file, console

# Log channel 'file' settings
log.file.type = file
log.file.destination = /var/log/siodb/siodb
log.file.max_file_size = 10M
log.file.exp_time = 1d
log.file.severity = info

# Log channel 'console' settings
log.console.type = console
log.console.destination = stdout
```

## Installation directory

The default installation directory is `/opt/siodb-<VERSION>`.
This is the home directory of Siodb for a given release Id.
You can find the Siodb binaries in `${SIODB_HOME}/bin`.

## Authentication

To authenticate yourself to the server, you must have [an active user created in Siodb
with an active access key](users.md).

## Start an instance of Siodb

To start and stop an instance of Siodb you must respect the following rules:

- Use the `siodb` binary.
- Be connected with the `siodb` user.
- Use the `--instance <instance_name>` parameter to tell Siodb which parameter file it must use.

```bash
$ /opt/siodb-<VERSION>/bin/siodb --help
Allowed options:
  -i [ --instance ] arg Instance name
  -d [ --daemon ]       Run as daemon
  -h [ --help ]         Produce help message
```

## Connect with the command line client

You can connect to an instance with the `siocli` client. There are two connection
modes, the local mode with a Unix socket and the remote mode over TCP/IP.

```bash
$ /opt/siodb-<VERSION>/bin/siocli --help
Allowed options:
  -a [ --admin ] arg                    Connect to given instance in the admin mode
  -H [ --host ] arg (=localhost)        Server host name or IP address
  -p [ --port ] arg (=50000)            Server port
  -k [ --keep-going ]                   Keep going if stdin is pipe or file and error occurred
  -i [ --identity-file ] arg (=/home/siodb/.ssh/id_rsa)
                                        Identity file (client private key)
  -u [ --user ] arg (=siodb)            User name
  -h [ --help ]                         Produce help message
```

## Connect to an instance with the admin mode

To connect to an instance of Siodb you use must respect the following rules:

- Use the `siocli` executable.
- Use the option `--admin` to connect locally. In the admin mode, the connection
is local and uses a Unix socket to communicate with Siodb.

## Connect to an instance over TCP/IP

To connect to a remote instance of Siodb, you must provide the network identifier of
the remote instance. This can be the hostname (`--host`) or an IP with the
port number (`--port`).

Example of a remote connection:

```bash
nico@siodb:~$ /opt/siodb-0.6/bin/siocli --host 192.168.56.50 --port 50000 --user nico --identity-file .ssh/id_rsa
Siodb client v.0.6.0
Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.

siocli> select name from sys_databases ;
Connected to 192.168.56.50:50000
NAME
--------------------
                 SYS

1 rows.
Command execution time: 45 ms.
```

If you don't provide the username and the identity file, `siocli` will use your current
OS username and the first private key found into the `.ssh` locate in your home. For instance:

```bash
nico@siodb:~$ /opt/siodb-0.6/bin/siocli --host 192.168.56.50 --port 50000
Siodb client v.0.6.0
Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.

siocli> select name from sys_databases ;
Connected to 192.168.56.50:50000
NAME
--------------------
                 SYS

1 rows.
Command execution time: 44 ms.
```








