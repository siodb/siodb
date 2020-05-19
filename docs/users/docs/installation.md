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
| CentOS                       | x86_64       | >=7.5   | Not Supported yet |
| RHEL                         | x86_64       | >=7.5   | Not Supported yet |
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

Download your distribution package on the official [website](https://siodb.io/getsiodb/).
There is also a script that installs everything properly for you with the last
version of Siodb available. To use that automation script, please go to [the
Quick start page](./../quick_start).

## Installation: Ubuntu 18.04

### Install dependencies

- Connect to your server as `root` or any privileged user
- Install the Debian packages:

```bash
apt-get update
apt install -y openssh-client openssl rlwrap
```

- Then, install the Siodb's dependency packages (available [here](https://siodb.io/getsiodb/)):

```bash
apt install -y ./antlr4_<VERSION>_x86_64.deb
apt install -y ./protobuf_<VERSION>_x86_64.deb
apt install -y ./boost_<VERSION>_x86_64.deb
apt install -y ./xxhash_<VERSION>_x86_64.deb
```

### Siodb installation

- Connect to your server as `root` or any privileged user
- Install the Debian package:

```bash
apt install -y ./siodb_<VERSION>_x86_64.deb
```
