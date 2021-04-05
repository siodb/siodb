# Encryption

When you want to encrypt data, the encryption happens at two levels: on data files
(encryption at rest) and on data in transit over a network (secure connection).
Siodb encrypts both by default. Hence you don't need to care about it. But if you
are know how it works, this section will tell you.

## Encryption at rest

Encryption at rest defines the encryption of the data in data files. The following
data files will are encrypted:

- Data block files
- Index files
- System table information data files

Siodb encrypts data files with the default `CIPHER_ID` at creation time. For instance, the
command below will create an encrypted database with the default `CIPHER_ID` configured in
the parameter file:

```sql
create database abcd ;
```

Default cipher can be specified in the Siodb instance config file with the following parameter:

- `encryption.default_cipher_id`

If this parameter is not explicitly specified, effective default cipher must be 'aes128'.

Siodb can use a different encryption algorithms to encrypt database keys. It is specified
by the configuration parameter `encryption.master_cipher_id`. If it is not specified explicitly,
Siodb uses the effective default cipher.

Siodb can also use a different encryption algorithms to system database. It is specified
by the following configuration parameter: `encryption.system_db_cipher_id`. If it is not specified
explicitly, Siodb uses the effective default cipher.

There are currently two ciphers available out-of-the-box:

- AES
- Camellia

You can eventually disable the encryption by using the `CIPHER_ID` `none`:

```sql
create database abcd with cipher_id = 'none' ;
```

Siodb stores the encryption keys for each database in the database directory.
Each database key is encrypted as well with a master encryption key. The master encryption
key `/etc/siodb/instances/<INSTANCE_NAME>/master_key`. Siodb reads this file at startup
time to get the master encryption key and decrypts the encryption key of each database.

Siodb identified the cipher with a `CIPHER_ID` which is a single string that includes:

- Encryption algorithm name
- Key size
- Block size, if multiple block sizes allowed.

Examples:

- `aes128` - AES with key size 128 bits.
- `aes256` - AES with key size 256 bits.

Siodb stores per-database cipher parameters in table `SYS_DATABASES`:

- `CIPHER_ID` - stores cipher identification string, as described above
- `CIPHER_KEY` - store encryption key

You can explicitly specify the cipher parameters in the `CREATE DATABASE` statement as follows:

```sql
create database abcd with
  cipher_id = 'aes128',
  cipher_key_seed = 'fksgksgjrekgjerkglerjg' ;
```

### Encryption details

Siodb generates the encryption key for the new database through cyclic hashing of the
entropy data pool, which includes the following elements:

- `CIPHER_KEY_SEED`
- Current system time
- At least 32 bytes from the system random number source (/dev/urandom).

The number of hashing cycles is determined randomly based on the additional 2 bytes,
read from the /dev/urandom, which are treated as a little-endian unsigned integer. That integer
is added 32768, and the result will be a number of hashing cycles to derive an encryption key.

Hash algorithm:

- SHA-256 for generation of keys up to 256 bits
- SHA-512 for generation of keys up to 512 bits

If required key size X is shorter than outcome of hash function, first X bits are taken as a key.

## secure connection

Siodb uses the [OpenSSL library](https://www.openssl.org/) to encrypt the data in sessions.
By default, the secure connection is enabled, and you can disable it in the
configuration file.

### Configuration

In most cases, if you use a packaged version of Siodb, the certificate and the private
key are already generated a installation time. This default certificate is valid for ten years.
But you may want to change the certificate and the private key.

To check the current configuration, inspect the Siodb configuration file:
`/etc/siodb/instances/siodb/config`.

```bash
# Should encrypted connection be used for client (yes(default)/no)
client.enable_encryption = yes

# Client connection OpenSSL certificate
client.tls_certificate = /etc/siodb/instances/siodb/cert.pem

# Client secure connection certificate/certificate chain private key
client.tls_private_key = /etc/siodb/instances/siodb/key.pem
```

As you can see in the above snippet, the encryption is enabled, and it uses a certificate
and a private key in PEM format.

If you want to generate your self-signed certificate and your private key,
you can do it with that one-command:

```bashs
openssl req \
       -newkey rsa:2048 -nodes -keyout /etc/siodb/instances/siodb/key.pem \
       -x509 -days 365 -out /etc/siodb/instances/siodb/cert.pem
```

If you have an existing CSR (Certificate Signing Requests) and a private key, you can use it.
We recommend [this article](https://www.digitalocean.com/community/tutorials/openssl-essentials-working-with-ssl-certificates-private-keys-and-csrs) for more details on how to manage your certificates with
Siodb.

### Disable secure connection

To disable secure connection, just set `client.enable_encryption` to `no` in the
configuration file `/etc/siodb/instances/siodb/config`:

```bash
# Should encrypted connection be used for client (yes(default)/no)
client.enable_encryption = no
```

When secure connection is disabled, you must explicitly tell `siocli` that you want to
create a plain text session this way:

```bash
$ /opt/siodb-0.6/bin/siocli --user root --plaintext
Siodb client v.0.10.0
Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.

siocli>
```

### How to verify that your data are encrypted on the network

A simple way to verify if encryption works are to view the traffic between `siocli` and Siodb.
I use the tool `tcpdump` to view those data:

```bashs
tcpdump -X -i enp0s8 port 50000
```

For more details on the option, we recommend [this page](https://www.cyberciti.biz/faq/tcpdump-capture-record-protocols-port/).

#### Encrypted data

```sql
siocli> select * from sys_databases ;

TRID                UUID                             NAME                             CIPHER_ID                        CIPHER_KEY
------------------- -------------------------------- -------------------------------- -------------------------------- --------------------------------
                  1 68ba038e-b704-2cb9-1d0d-b9186...                             SYS                          aes128  0x61616161616161616161616161...

1 rows.
```

```bash
        0x0000:  4500 006d 7a5a 4000 4006 ce74 c0a8 3839  E..mzZ@.@..t..89
        0x0010:  c0a8 3832 a11a c350 f322 8db0 0d42 3cdd  ..82...P."...B<.
        0x0020:  8018 01f5 9f3b 0000 0101 080a 41e0 75fd  .....;......A.u.
        0x0030:  c460 2c96 1703 0300 345b 2142 0bd9 99c5  .`,.....4[!B....
        0x0040:  d41d 9f68 85ae 3c6e 6e5b 7fa6 130e 9a76  ...h..<nn[.....v
        0x0050:  74b7 ae5e eb6c 2c20 92fa eba6 d6cf 2b0f  t..^.l,.......+.
        0x0060:  4372 c749 42c6 c068 5d20 07c4 f7         Cr.IB..h]....
        0x0000:  4500 008d 99aa 4000 4006 af04 c0a8 3832  E.....@.@.....82
        0x0010:  c0a8 3839 c350 a11a 0d42 3cdd f322 8de9  ..89.P...B<.."..
        0x0020:  8018 01f9 f23b 0000 0101 080a c460 6693  .....;.......`f.
        0x0030:  41e0 75fd 1703 0300 54f3 171a afbc 7d79  A.u.....T.....}y
        0x0040:  4efc a690 3fb9 5019 c3d3 d807 67e9 224a  N...?.P.....g."J
        0x0050:  1627 ffef 8923 2f50 a550 9439 4d5f ff53  .'...#/P.P.9M_.S
        0x0060:  c322 b5d2 1160 17e8 5aad 2624 1716 01d7  ."...`..Z.&$....
        0x0070:  2a38 078d 1902 cbb7 f1ba c386 a63a b313  *8...........:..
        0x0080:  52b9 5132 75e6 1a5c 497f a9c0 5a         R.Q2u..\I...Z
        0x0000:  4500 0034 7a5b 4000 4006 ceac c0a8 3839  E..4z[@.@.....89
        0x0010:  c0a8 3832 a11a c350 f322 8de9 0d42 3d36  ..82...P."...B=6
        0x0020:  8010 01f5 7045 0000 0101 080a 41e0 7602  ....pE......A.v.
        0x0030:  c460 6693                                .`f.
        0x0000:  4500 008e 99ab 4000 4006 af02 c0a8 3832  E.....@.@.....82
        0x0010:  c0a8 3839 c350 a11a 0d42 3d36 f322 8de9  ..89.P...B=6."..
        0x0020:  8018 01f9 f23c 0000 0101 080a c460 6694  .....<.......`f.
        0x0030:  41e0 7602 1703 0300 555e 6443 daa7 5a36  A.v.....U^dC..Z6
        0x0040:  1d41 b763 1f54 01a9 6490 138c 3caf 7cf3  .A.c.T..d...<.|.
        0x0050:  e78e 8c51 590b f69a 5e19 4adb f46e 8b88  ...QY...^.J..n..
        0x0060:  57de ea53 328c dd81 fc93 f3b5 6111 17cb  W..S2.......a...
        0x0070:  28e7 f614 45ad 5610 635a 0509 6b35 24d5  (...E.V.cZ..k5$.
        0x0080:  cb84 39b1 1cc0 8183 5bc6 cfc4 f067       ..9.....[....g
        0x0000:  4500 0034 7a5c 4000 4006 ceab c0a8 3839  E..4z\@.@.....89
        0x0010:  c0a8 3832 a11a c350 f322 8de9 0d42 3d90  ..82...P."...B=.
        0x0020:  8010 01f5 6fe9 0000 0101 080a 41e0 7603  ....o.......A.v.
        0x0030:  c460 6694                                .`f.
```

#### Unencrypted data

```sql
siocli> select * from sys_databases ;

TRID                UUID                             NAME                             CIPHER_ID                        CIPHER_KEY
------------------- -------------------------------- -------------------------------- -------------------------------- --------------------------------
                  1 68ba038e-b704-2cb9-1d0d-b9186...                             SYS                          aes128  0x61616161616161616161616161...

1 rows.
```

```bash
        0x0000:  4500 0057 bbd5 4000 4006 8d0f c0a8 3839  E..W..@.@.....89
        0x0010:  c0a8 3832 a118 c350 2a33 52ff fcea f7f5  ..82...P*3R.....
        0x0020:  8018 01f5 0045 0000 0101 080a 41df dd12  .....E......A...
        0x0030:  c45f 06a8 0121 0805 121d 7365 6c65 6374  ._...!....select
        0x0040:  202a 2066 726f 6d20 7379 735f 6461 7461  .*.from.sys_data
        0x0050:  6261 7365 7320 3b                        bases.;
        0x0000:  4500 0077 57bf 4000 4006 f105 c0a8 3832  E..wW.@.@.....82
        0x0010:  c0a8 3839 c350 a118 fcea f7f5 2a33 5322  ..89.P......*3S"
        0x0020:  8018 01fb f225 0000 0101 080a c45f cda7  .....%......._..
        0x0030:  41df dd12 0241 0805 1a08 0a04 5452 4944  A....A......TRID
        0x0040:  1008 1a08 0a04 5555 4944 100b 1a08 0a04  ......UUID......
        0x0050:  4e41 4d45 100b 1a0d 0a09 4349 5048 4552  NAME......CIPHER
        0x0060:  5f49 4410 0b1a 0e0a 0a43 4950 4845 525f  _ID......CIPHER_
        0x0070:  4b45 5910 0d30 01                        KEY..0.
        0x0000:  4500 0034 bbd6 4000 4006 8d31 c0a8 3839  E..4..@.@..1..89
        0x0010:  c0a8 3832 a118 c350 2a33 5322 fcea f838  ..82...P*3S"...8
        0x0020:  8010 01f5 fb2b 0000 0101 080a 41df dd16  .....+......A...
        0x0030:  c45f cda7                                ._..
        0x0000:  4500 0078 57c0 4000 4006 f103 c0a8 3832  E..xW.@.@.....82
        0x0010:  c0a8 3839 c350 a118 fcea f838 2a33 5322  ..89.P.....8*3S"
        0x0020:  8018 01fb f226 0000 0101 080a c45f cda7  .....&......._..
        0x0030:  41df dd16 4201 2436 3862 6130 3338 652d  A...B.$68ba038e-
        0x0040:  6237 3034 2d32 6362 392d 3164 3064 2d62  b704-2cb9-1d0d-b
        0x0050:  3931 3836 3463 3831 3963 6403 5359 5306  91864c819cd.SYS.
        0x0060:  6165 7331 3238 1061 6161 6161 6161 6161  aes128.aaaaaaaaa
        0x0070:  6161 6161 6161 0a00                      aaaaaa..
        0x0000:  4500 0034 bbd7 4000 4006 8d30 c0a8 3839  E..4..@.@..0..89
        0x0010:  c0a8 3832 a118 c350 2a33 5322 fcea f87c  ..82...P*3S"...|
        0x0020:  8010 01f5 fae7 0000 0101 080a 41df dd16  ............A...
        0x0030:  c45f cda7                                ._..
```
