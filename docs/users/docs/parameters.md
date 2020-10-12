# Siodb parameters

## admin_connection_listener_backlog

Backlog value for the admin connection listener.

**Example:**

```init
admin_connection_listener_backlog = 10
```

## client.enable_encryption

Should encrypted connection be used for client (yes(default)/no).

**Example:**

```init
client.enable_encryption = yes
```

## client.tls_certificate

Client connection OpenSSL certificate.

**Example:**

```init
client.tls_certificate = /etc/siodb/instances/siodb000/cert.pem
```

## client.tls_certificate_chain

Client connection OpenSSL certificate chain.
If both tls_certificate and tls_certificate_chain is set tls_certificate_chain is used.

**Example:**

```init
client.tls_certificate_chain = /etc/siodb/instances/siodb000/certChain.pem
```

## client.tls_private_key

Client secure connection certificate/certificate chain private key.

**Example:**

```init
client.tls_private_key = /etc/siodb/instances/siodb000/key.pem
```

## data_dir

Data storage directory.

**Example:**

```init
data_dir = /var/lib/siodb/siodb000/data
```

## enable_rest_server

Enables or disables REST Server service.

**Example:**

```init
enable_rest_server = yes
```

## encryption.default_cipher_id

Encryption default cipher id (aes128 is used if not set).

**Example:**

```init
encryption.default_cipher_id = aes128
```

## encryption.master_key

Encryption key used to encrypt and decrypt instance level data
/etc/siodb/instances/{INSTANCE_NAME}/master_key is used if not set.

**Example:**

```init
encryption.master_key = /etc/siodb/instances/siodb000/master_key
```

## encryption.system_db_cipher_id

Encryption algorithm used to encrypt system database.
Encryption.default_cipher_id is used if not set.

**Example:**

```init
encryption.system_db_cipher_id = aes128
```

## iomgr.block_cache_capacity

Capacity of the block cache (in 10M blocks).

**Example:**

```init
iomgr.block_cache_capacity = 103
```

## iomgr.database_cache_capacity

Database cache capacity.

**Example:**

```init
iomgr.database_cache_capacity = 100
```

## iomgr.ipv4_port

IO Manager listening port for IPv4 client connections.
0 means do not listen.

**Example:**

```init
iomgr.ipv4_port = 50001
```

## iomgr.ipv6_port

IO Manager listening port for IPv6 client connections.
0 means do not listen.

**Example:**

```init
iomgr.ipv6_port = 0
```

## iomgr.max_json_payload_size

Maximum JSON payload size in the REST request in kilobytes.
Suffixes k, K, m, M, g, G switch measure unit to KiB, MiB and GiB respectively.

**Example:**

```init
iomgr.max_json_payload_size = 1024
```

## iomgr.rest.ipv4_port

IO Manager listening port for IPv4 REST connections.
0 means do not listen.

**Example:**

```init
iomgr.rest.ipv4_port = 50002
```

## iomgr.rest.ipv6_port

IO Manager listening port for IPv6 REST connections.
0 means do not listen.

**Example:**

```init
iomgr.rest.ipv6_port = 0
```

## iomgr.table_cache_capacity

Table cache capacity.

**Example:**

```init
iomgr.table_cache_capacity = 100
```

## iomgr.worker_thread_number

IO Manager worker thead number. 0 means do not listen.

**Example:**

```init
iomgr.worker_thread_number = 2
```

## ipv4_port

Listening port for IPv4 client connections.
0 means do not listen.

**Example:**

```init
ipv4_port = 50000
```

## ipv6_port

Listening port for IPv6 client connections.
0 means do not listen.

**Example:**

```init
ipv6_port = 0
```

## log.console.destination

Log channel 'console' settings.

**Example:**

```init
log.console.destination = stdout
```

## log.console.severity

Log channel 'console' settings.

**Example:**

```init
log.console.severity = info
```

## log.console.type

Log channel 'console' settings.

**Example:**

```init
log.console.type = console
```

## log.file.destination

Log channel 'file' settings.

**Example:**

```init
log.file.destination = /var/log/siodb/siodb000
```

## log.file.exp_time

Log channel 'file' settings.

**Example:**

```init
log.file.exp_time = 1d
```

## log.file.max_file_size

Log channel 'file' settings.

**Example:**

```init
log.file.max_file_size = 10M
```

## log.file.severity

Log channel 'file' settings.

**Example:**

```init
log.file.severity = info
```

## log.file.type

Log channel 'file' settings.

**Example:**

```init
log.file.type = file
```

## log_channels

Log channel 'file' settings.

**Example:**

```init
log_channels = file, console
```

## max_admin_connections

Max. number of admin connections

**Example:**

```init
max_admin_connections = 10
```

## max_user_connections

Max. number of user connections.

**Example:**

```init
max_user_connections = 100
```

## rest_server.chunk_size

HTTP chunk size in bytes. Suffixes `k`, `K`, `m`, `M` change units
to the kilobytes and megabytes respectively.

**Example:**

```init
rest_server.chunk_size = 64K
```

## rest_server.iomgr_read_timeout

Read deadline timeout in seconds for connections to IOMgr

**Example:**

```init
rest_server.iomgr_read_timeout = 10
```

## rest_server.ipv4_http_port

IPv4 HTTP port number.
0 means do not listen.

**Example:**

```init
rest_server.ipv4_http_port = 50080
```

## rest_server.ipv4_https_port

IPv4 HTTPS port number.
0 means do not listen.

**Example:**

```init
rest_server.ipv4_https_port = 50443
```

## rest_server.ipv6_http_port

IPv6 HTTP port number.
0 means do not listen.

**Example:**

```init
rest_server.ipv6_http_port = 0
```

## rest_server.ipv6_https_port

IPv6 HTTPS port number.
0 means do not listen.

**Example:**

```init
rest_server.ipv6_https_port = 0
```

## rest_server.request_payload_buffer_size

Maximum buffer size to process request payload to IOMgr. Suffixes `k`, `K`, `m`, `M` change units
to the kilobytes and megabytes respectively.

**Example:**

```init
rest_server.request_payload_buffer_size = 2k
```

## rest_server.tls_certificate

Path to the TLS certificate file.

**Example:**

```init
rest_server.tls_certificate = /etc/siodb/instances/siodb000/cert.pem
```

## rest_server.tls_certificate_chain

Path to the TLS certificate chain file.
If both rest_server.tls_certificate and tls_certificate_chain are set,
then rest_server.tls_certificate_chain is used.

**Example:**

```init
rest_server.tls_certificate_chain = /etc/siodb/instances/siodb000/certChain.pem
```

## rest_server.tls_private_key

Path to the TLS private key file.

**Example:**

```init
rest_server.tls_private_key = /etc/siodb/instances/siodb000/key.pem
```

## user_connection_listener_backlog

Backlog value for the user connection listener.

**Example:**

```init
user_connection_listener_backlog = 10
```
