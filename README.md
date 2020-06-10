![alt text](https://siodb.io/wp-content/uploads/2020/05/SIODB_Logo_Editable_half_landscape_small_logo_site.png)

# Siodb

A relational SQL database that secures your data, manages personal data life-cycle, and automates operational tasks. Feature out-of-the-box:

- Transparent encryption
- Standard SQL
- Data versioning
- Data model versioning
- Automatic indexing
- Replication
- Distribution
- Microservice friendly
- High performance
- High security
- Transparent compression
- Automatic In memory
- Automatic data identification
- Partitioned columns storage
- Effortless administration

**Current status:** Alpha version

## Quick Start

### Docker

```bash
docker run -p 127.0.0.1:50000:50000/tcp --name siodb siodb/siodb
```

### Cloud

[![Deploy to Hidora](https://raw.githubusercontent.com/siodb/siodb-jelastic/master/images/deploy-to-hidora.png)](https://siodb.hidora.com)

*Free Trial. Only requires an email address.*

### Connect to the instance in the container

```bash
siodb@ea7ad13791c1:/$ siocli --host localhost --port 50000 --user root --identity-file ~/.ssh/id_rsa
Siodb client v.0.6.0
Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.

siocli> show databases ;
UUID                             NAME
-------------------------------- --------------------------------
68ba038e-b704-2cb9-1d0d-b9186...                             SYS

1 rows.
```

## Quick Start (Linux)

### Get the bootstrap script and run it

```bash
wget https://siodb.io/packages/InstallSiodb.sh
chmod u+x ./InstallSiodb.sh
sudo ./InstallSiodb.sh
```

#### Connect to the instance

- Connect to your server as `siodb`

```bash
sudo su - siodb
siocli --host localhost --port 50000 --user root --identity-file ~/.ssh/id_rsa
```

Reference [here](https://docs.siodb.io/quick_start/).

## Developer's Corner

### Contributing

Go to the contribution page 👉 [Here](CONTRIBUTING.md).

### Drivers

| Language | Link                                                 |
| -------- | :--------------------------------------------------- |
| Golang   | [Github](https://github.com/siodb/siodb-go-driver)   |
| Rust     | [Github](https://github.com/siodb/siodb-rust-driver) |
| Java     | [Github](https://github.com/siodb/siodb-jdbc-driver) |
| Python   | [Github](https://github.com/siodb/siodb-python-driver) |
| C++      | [Github](https://github.com/siodb/siodb-cxx-driver) |
| .NET     | [Github](https://github.com/siodb/siodb-dotnet-driver) |

### Development resources

The resources to integrate Siodb in your development are
available [here](https://docs.siodb.io/development).

## Building Siodb

To build Siodb from the source code, please follow the guidelines according to your Linux
distribution [here](docs/dev/Build.md).

## Documentation

We write the documentation in Markdow and it is available in the folder `docs/users/docs`.
If you prefer a more user friendly format, the same documentation is
available online [here]( https://docs.siodb.io).

## Support

- Report your issue 👉 [here](https://github.com/siodb/siodb/issues/new).
- Ask a question 👉 [here](https://stackoverflow.com/questions/tagged/siodb).
- Siodb Slack space 👉 [here](https://join.slack.com/t/siodb-squad/shared_invite/zt-e766wbf9-IfH9WiGlUpmRYlwCI_28ng).

Whatever you would like to share with us, we are always prepared to listen: code@siodb.io.

## Support Siodb

Do you like this project? Tell it by clicking the star 🟊 on the top right of this page ☝☝

## Follow Siodb

- [Twitter](https://twitter.com/Sio_db)
- [Linkedin](https://www.linkedin.com/company/siodb)

## License

Siodb is free, and the source is available under the AGPL v3. Siodb uses
bundled dependencies for which you can find the license in the NOTICE file
in this repository's top-level directory.

## Useful References

- Official website of the project 👉 [here](https://siodb.io).
- [SQL99 Complete, Really](https://crate.io/docs/sql-99/en/latest/index.html)
- [Linux Containers](https://linuxcontainers.org/lxd/getting-started-cli/)
