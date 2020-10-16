# Installation

## Requirements

### Hardware

#### Processor

Siodb runs on the x86 processor architecture 64 bits.
You should plan your CPU requirement according to the expected database load.
If you can't figure out now, start with 8 threads. Then add
monitoring to forecast your further CPU needs.

#### Disk space

Siodb uses three distinct spaces:

- **Software space:**
this space is the location where you install Siodb. You can dedicate at least 500MB.

- **Data space:**
Siodb uses this space for business data. You can change this path with the parameter `data_dir`.
There must be enough space in that directory so you can store all the data for your database.
You should monitor this space to avoid issues with Siodb.

- **Log file space:**
This space is the location where Siodb logs its activity. You can dedicate at least 1GB.
You can configure log file destination, rotation and retention policy with `log.*` parameters.

#### Memory

You should plan your memory requirement according to the expected database load.
If you can't figure out now, you can allocate 4GB. Then add monitoring
to forecast your further memory needs.

### Software

#### Operating system

Siodb aims to support the following operating systems:

| OS                           | Architecture | Release | Status            |
| ---------------------------- | ------------ | ------- | ----------------- |
| Ubuntu                       | x86_64       | >=18.04 | Supported         |
| Debian                       | x86_64       | >=10    | Supported         |
| CentOS                       | x86_64       | >=7.8   | Supported         |
| RHEL                         | x86_64       | >=7.8   | Supported         |
| Alpine                       | x86_64       | >=3.10  | Not Supported yet |
| openSUSE (Leap)              | x86_64       | >=15    | Not Supported yet |
| SUSE Linux Enterprise Server | x86_64       | >=15    | Not Supported yet |

#### User and group

The `root` OS user owns the Siodb binaries.

Siodb runs with the `siodb` OS user and it owns the data files and configuration files.

Siodb checks that your current OS user belongs to the `siodb` OS group when executing administrative tasks.
You may want to perform administrative tasks with your OS user.

If you choose the **manual installation from the source code**,
You must create the Siodb OS user and the OS group to run Siodb:

- OS user: `siodb`
- OS group: `siodb`

#### Data directory

The data directory is a directory that Siodb will use to store its data.
You can configure that directory in the configuration file with the parameter `data_dir`.

This directory must belong to the `siodb` OS user with the read, write, and execute permissions.
Only user belonging to the `siodb` OS group should be able to read, write and list files from that directory.

## Linux Packages

Download your the packages for your distribution here
[website](https://www.siodb.io/packages/siodb_linux_packages.html).
There is also an automation script that installs everything for you with the last
version of Siodb available. To use the automation script, please go to [the
Quick start page](./../quick_start).

## Installation per OS

### Ubuntu 18.04

#### Install dependencies as root

```bash
apt install -y openssh-client openssl rlwrap
apt install -y ./siodb-protobuf_<VERSION>-ubuntu1804_amd64.deb
apt install -y ./siodb-antlr4-runtime_<VERSION>-ubuntu1804_amd64.deb
apt install -y ./siodb-xxhash_<VERSION>-ubuntu1804_amd64.deb
```

Siodb's dependency packages are available [here](https://www.siodb.io/packages/siodb_linux_packages.html).

#### Siodb installation

```bash
apt install -y ./siodb_<VERSION>-ubuntu1804_amd64.deb
```

### Ubuntu 20.04

#### Install dependencies as root

```bash
apt install -y openssh-client openssl rlwrap
apt install -y ./siodb-protobuf_<VERSION>-ubuntu2004_amd64.deb
apt install -y ./siodb-antlr4-runtime_<VERSION>-ubuntu2004_amd64.deb
apt install -y ./siodb-xxhash_<VERSION>-ubuntu2004_amd64.deb
```

Siodb's dependency packages are available [here](https://www.siodb.io/packages/siodb_linux_packages.html).

#### Siodb installation

```bash
apt install -y ./siodb_<VERSION>-ubuntu2004_amd64.deb
```

### Debian 10

#### Install dependencies as root

```bash
apt install -y openssh-client openssl rlwrap
apt install -y ./siodb-protobuf_<VERSION>-debian10_amd64.deb
apt install -y ./siodb-antlr4-runtime_<VERSION>-debian10_amd64.deb
apt install -y ./siodb-xxhash_<VERSION>-debian10_amd64.deb
```

Siodb's dependency packages are available [here](https://www.siodb.io/packages/siodb_linux_packages.html).

#### Siodb installation

```bash
apt install -y ./siodb_<VERSION>-debian10_amd64.deb
```

### Centos 7 / RHEL 7

#### Install dependencies as root

```bash
yum install -y openssh-clients rlwrap
yum install -y ./siodb-openssl-<VERSION>-1.el7.x86_64.rpm
yum install -y ./siodb-protobuf-<VERSION>-1.el7.x86_64.rpm
yum install -y ./siodb-antlr4-runtime-<VERSION>.el7.x86_64.rpm
yum install -y ./siodb-xxhash-<VERSION>-1.el7.x86_64.rpm
```

Siodb's dependency packages are available [here](https://www.siodb.io/packages/siodb_linux_packages.html).

#### Siodb installation

```bash
yum install -y ./siodb-<VERSION>-1.el7.x86_64.rpm
```

### Centos 8 / RHEL 8

#### Install dependencies as root

```bash
yum install -y openssh-clients openssl rlwrap
yum install -y ./siodb-protobuf-<VERSION>-1.el8.x86_64.rpm
yum install -y ./siodb-antlr4-runtime-<VERSION>.el8.x86_64.rpm
yum install -y ./siodb-xxhash-<VERSION>-1.el8.x86_64.rpm
```

Siodb's dependency packages are available [here](https://www.siodb.io/packages/siodb_linux_packages.html).

#### Siodb installation

```bash
yum install -y ./siodb-<VERSION>-1.el8.x86_64.rpm
```
