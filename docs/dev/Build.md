# Building Siodb

- [Environment Preparation](#environment-preparation)
  - [Ubuntu 18.04 LTS](#ubuntu-1804-lts)
  - [Ubuntu 20.04 LTS](#ubuntu-2004-lts)
  - [Debian 10](#debian-10)
  - [CentOS 7](#centos-7)
  - [CentOS 8](#centos-8)
  - [RHEL 7](#rhel-7)
  - [RHEL 8](#rhel-8)
- [All Systems](#all-systems)
  - [Install Go](#install-go)
  - [Install boost-pretty-printer GDB Extension](#install-boost-pretty-printer-gdb-extension)
- [Building Third-Party Libraries](#building-third-party-libraries)
  - [C and C++ Libraries](#c-and-c-libraries)
  - [Go Libraries](#go-libraries)
- [System Setup (Debian, Ubuntu, CentOS, RHEL, SuSE, SLES)](#system-setup-debian-ubuntu-centos-rhel-suse-sles)
- [Compiling Siodb](#compiling-siodb)
- [Running Siodb](#running-siodb)

## Environment Preparation

### Ubuntu 18.04 LTS

Run following commands:

```bash
cd $HOME

# Required tools and libraries
sudo apt install -y autoconf automake build-essential cmake doxygen gdb git \
    graphviz gcc-8 g++-8 libboost1.65-dev libboost-iostreams1.65-dev libboost-log1.65-dev \
    libboost-program-options1.65-dev libcurl4-openssl-dev libtool libssl-dev \
    lsb-release openjdk-11-jdk-headless python pkg-config uuid-dev \
    clang-format-10 ubuntu-dbgsym-keyring

# Set up alternatives for the clang-format
sudo update-alternatives --install /usr/bin/clang-format clang-format \
    /usr/lib/llvm-10/bin/clang-format 1
sudo update-alternatives --install /usr/bin/git-clang-format git-clang-format \
    /usr/lib/llvm-10/bin/git-clang-format 1
sudo update-alternatives --set clang-format /usr/lib/llvm-10/bin/clang-format
sudo update-alternatives --set git-clang-format \
    /usr/lib/llvm-10/bin/git-clang-format
```

Now, proceed to the section [All Systems](#all-systems).

### Ubuntu 20.04 LTS

Run following commands:

```bash
cd $HOME

# Required tools and libraries
sudo apt install -y autoconf automake build-essential cmake doxygen gdb git \
    graphviz libboost1.71-dev libboost-iostreams1.71-dev libboost-log1.71-dev \
    libboost-program-options1.71-dev libcurl4-openssl-dev libtool libssl-dev \
    lsb-release openjdk-11-jdk-headless pkg-config python2 uuid-dev \
    clang-format-10 ubuntu-dbgsym-keyring

# Set up alternatives for the clang-format
sudo update-alternatives --install /usr/bin/clang-format clang-format \
    /usr/lib/llvm-10/bin/clang-format 1
sudo update-alternatives --install /usr/bin/git-clang-format git-clang-format \
    /usr/lib/llvm-10/bin/git-clang-format 1
sudo update-alternatives --set clang-format /usr/lib/llvm-10/bin/clang-format
sudo update-alternatives --set git-clang-format /usr/lib/llvm-10/bin/git-clang-format

# Link Python 2 (required by Google Test fuse script)
sudo ln -s /usr/bin/python2 /usr/bin/python
```

Now, proceed to the section [All Systems](#all-systems).

### Debian 10

Run following commands:

```bash
cd $HOME

# Required tools and libraries
sudo apt install -y autoconf automake build-essential cmake doxygen gdb git \
    graphviz libboost1.67-dev libboost-iostreams1.67-dev libboost-log1.67-dev \
    libboost-program-options1.67-dev libcurl4-openssl-dev libtool libssl-dev \
    lsb-release openjdk-11-jdk-headless pkg-config python2 uuid-dev wget

# Install clang-9. This one is for SLES, but works on the Debian 10 too.
sudo apt install -y libncurses5
mkdir /tmp/getllvm
cd /tmp/getllvm
wget http://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-linux-sles11.3.tar.xz
tar --no-same-owner -xaf clang+llvm-9.0.0-x86_64-linux-sles11.3.tar.xz
sudo mkdir -p /usr/local/lib
sudo mv -f clang+llvm-9.0.0-x86_64-linux-sles11.3 /usr/local/lib
cd $HOME
rm -rf /tmp/getllvm

# Set up alternatives for the clang-format
sudo update-alternatives --install /usr/bin/clang-format clang-format \
    /usr/local/lib/clang+llvm-9.0.0-x86_64-linux-sles11.3/bin/clang-format 1
sudo update-alternatives --install /usr/bin/git-clang-format git-clang-format \
    /usr/local/lib/clang+llvm-9.0.0-x86_64-linux-sles11.3/bin/git-clang-format 1
sudo update-alternatives --set clang-format \
  /usr/local/lib/clang+llvm-9.0.0-x86_64-linux-sles11.3/bin/clang-format
sudo update-alternatives --set git-clang-format \
  /usr/local/lib/clang+llvm-9.0.0-x86_64-linux-sles11.3/bin/git-clang-format

# Link Python 2 (required by Google Test fuse script)
sudo ln -s /usr/bin/python2 /usr/bin/python
```

Now, proceed to the section [All Systems](#all-systems).

### CentOS 7

Run following commands:

```bash
cd $HOME

# Enable additional repositories
sudo yum install -y \
    https://packages.endpoint.com/rhel/7/os/x86_64/endpoint-repo-1.8-1.x86_64.rpm \
    https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm

# Enable software collections
sudo yum install -y centos-release-scl

# Update your system
sudo yum -y update

# Uninstall any existing git
sudo yum remove git*

# Install latest git
sudo yum install -y git

# Install required tools and libraries
sudo yum install -y autoconf automake boost169-devel cmake3 curl \
    devtoolset-8-toolchain doxygen gcc gcc-c++ java-1.8.0-openjdk-headless \
    libatomic libcurl-devel libtool libuuid-devel openssl-devel pkgconfig \
    python redhat-lsb uuid-devel wget which zlib-devel

sudo alternatives --install /usr/local/bin/cmake cmake /usr/bin/cmake 10 \
--slave /usr/local/bin/ctest ctest /usr/bin/ctest \
--slave /usr/local/bin/cpack cpack /usr/bin/cpack \
--slave /usr/local/bin/ccmake ccmake /usr/bin/ccmake \
--family cmake

sudo alternatives --install /usr/local/bin/cmake cmake /usr/bin/cmake3 20 \
--slave /usr/local/bin/ctest ctest /usr/bin/ctest3 \
--slave /usr/local/bin/cpack cpack /usr/bin/cpack3 \
--slave /usr/local/bin/ccmake ccmake /usr/bin/ccmake3 \
--family cmake

# Install clang-9. This one is for SLES, but works for the CentOS 7 too.
mkdir /tmp/getllvm
cd /tmp/getllvm
wget http://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-linux-sles11.3.tar.xz
tar --no-same-owner -xaf clang+llvm-9.0.0-x86_64-linux-sles11.3.tar.xz
rm -f clang+llvm-9.0.0-x86_64-linux-sles11.3.tar.xz
mv -f clang+llvm-9.0.0-x86_64-linux-sles11.3 /usr/local/lib64
sudo alternatives --install /usr/local/bin/clang-format clang-format \
    /usr/local/lib64/clang+llvm-9.0.0-x86_64-linux-sles11.3/bin/clang-format 20 \
    --slave /usr/local/bin/git-clang-format git-clang-format \
    /usr/local/lib64/clang+llvm-9.0.0-x86_64-linux-sles11.3/bin/git-clang-format
cd $HOME
rm -rf /tmp/getllvm


# Permanently tell ldconfig to scan /usr/local/lib when updating cache
sudo /bin/sh -c 'echo "/usr/local/lib" >/etc/ld.so.conf.d/usr-local-lib.conf'
sudo ldconfig

# Create links for the libatomic - required by Oat++
sudo ln -s /usr/lib/libatomic.so.1.0.0 /usr/lib/libatomic.so
sudo ln -s /usr/lib64/libatomic.so.1.0.0 /usr/lib64/libatomic.so
```

Now, proceed to the section [All Systems](#all-systems).

### CentOS 8

Run following commands:

```bash
cd $HOME

# Enable additional repositories
sudo dnf install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm

# Update your system
sudo dnf -y update

# Install required tools and libraries
sudo dnf install -y autoconf automake boost-devel clang cmake curl gcc gcc-c++ \
    git-clang-format java-11-openjdk-headless libatomic libcurl-devel libtool \
    libuuid-devel openssl-devel python2 pkgconfig redhat-lsb wget which \
    zlib-devel

# Link Python 2 (required by Google Test fuse script)
sudo ln -s /usr/bin/python2 /usr/bin/python
```

Now, proceed to the section [All Systems](#all-systems).

### RHEL 7

Run following commands:

```bash
cd $HOME

# Enable additional repositories
sudo yum install -y \
    https://packages.endpoint.com/rhel/7/os/x86_64/endpoint-repo-1.7-1.x86_64.rpm \
    https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm

# Enable software collections
sudo yum-config-manager --enable rhel-server-rhscl-7-rpms

# Update your system
sudo yum -y update

# Uninstall any existing git
sudo yum remove git*

# Install latest git
sudo yum install -y git

# Install required tools and libraries
sudo yum install -y autoconf automake boost169-devel cmake3 curl \
    devtoolset-8-toolchain doxygen gcc gcc-c++ java-1.8.0-openjdk-headless \
    libatomic libcurl-devel lubtool libuuid-devel openssl-devel pkgconfig \
    python redhat-lsb uuid-devel wget which zlib-devel

sudo alternatives --install /usr/local/bin/cmake cmake /usr/bin/cmake 10 \
--slave /usr/local/bin/ctest ctest /usr/bin/ctest \
--slave /usr/local/bin/cpack cpack /usr/bin/cpack \
--slave /usr/local/bin/ccmake ccmake /usr/bin/ccmake \
--family cmake

sudo alternatives --install /usr/local/bin/cmake cmake /usr/bin/cmake3 20 \
--slave /usr/local/bin/ctest ctest /usr/bin/ctest3 \
--slave /usr/local/bin/cpack cpack /usr/bin/cpack3 \
--slave /usr/local/bin/ccmake ccmake /usr/bin/ccmake3 \
--family cmake

# Install clang-9. This one is for SLES, but works for the RHEL 7 too.
mkdir /tmp/getllvm
cd /tmp/getllvm
wget http://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-linux-sles11.3.tar.xz
tar --no-same-owner -xaf clang+llvm-9.0.0-x86_64-linux-sles11.3.tar.xz
rm -f clang+llvm-9.0.0-x86_64-linux-sles11.3.tar.xz
mv -f clang+llvm-9.0.0-x86_64-linux-sles11.3 /usr/local/lib64
sudo alternatives --install /usr/local/bin/clang-format clang-format \
    /usr/local/lib64/clang+llvm-9.0.0-x86_64-linux-sles11.3/bin/clang-format 20 \
    --slave /usr/local/bin/git-clang-format git-clang-format \
    /usr/local/lib64/clang+llvm-9.0.0-x86_64-linux-sles11.3/bin/git-clang-format
cd $HOME
rm -rf /tmp/getllvm

# Permanently tell ldconfig to scan /usr/local/lib when updating cache
sudo /bin/sh -c 'echo "/usr/local/lib" >/etc/ld.so.conf.d/usr-local-lib.conf'
sudo ldconfig

# Create links for the libatomic - required by Oat++
sudo ln -s /usr/lib/libatomic.so.1.0.0 /usr/lib/libatomic.so
sudo ln -s /usr/lib64/libatomic.so.1.0.0 /usr/lib64/libatomic.so
```

Now, proceed to the section [All Systems](#all-systems).

### RHEL 8

Run following commands:

```bash
cd $HOME

# Enable additional repositories
sudo dnf install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm

# Update your system
sudo dnf -y update

# Install required tools and libraries
sudo dnf install -y autoconf automake boost-devel clang cmake curl gcc gcc-c++ \
    git-clang-format java-11-openjdk-headless libatomic libcurl-devel libtool \
    libuuid-devel openssl-devel python2 pkgconfig redhat-lsb wget which \
    zlib-devel

# Link Python 2 (required by Google Test fuse script)
sudo ln -s /usr/bin/python2 /usr/bin/python
```

Now, proceed to the section [All Systems](#all-systems).

## All Systems

### Install Go

Run following commands:

```bash
cd /tmp
wget https://golang.org/dl/go1.15.2.linux-amd64.tar.gz
tar xaf go1.15.2.linux-amd64.tar.gz
mv go go-1.15
sudo mv go-1.15 /usr/local
```

### Install boost-pretty-printer GDB Extension

For better debugging experience, it is recommended to install
[Boost Pretty Printers](https://github.com/ruediger/Boost-Pretty-Printer):

1. Run following commands

    ```shell
    mkdir -p ~/.local/share
    cd ~/.local/share
    git clone git://github.com/ruediger/Boost-Pretty-Printer.git
    ```

2. Add following lines to the file `~/.gdbinit` (create it if it doesn't exist yet):

    ```text
    python
    import sys
    sys.path.insert(1, '~/.local/share/Boost-Pretty-Printer')
    import boost
    boost.register_printers(boost_version=(x,y,z))
    end
    ```

Now, proceed to the section [Building Third-Party Libraries](#building-third-party-libraries).

## Building Third-Party Libraries

### C and C++ Libraries

Change current directory to the root of siodb Git repository and execute following commands:

**NOTE:** Adjust make parameter `-j4` to number of CPUs/cores available on the your build machine.

```bash

# Clone Siodb repository or your Siodb fork, like this:
mkdir ~/projects
cd ~/projects
git clone git@github.com:siodb/siodb.git

# Enter repository root directory
cd siodb

# Install source code formatting hook for git
cp -fv tools/git/clang-format.hook .git/hooks/pre-commit

# CentOS 7/RHEL 7 ONLY: Enable devtoolset-8
scl enable devtoolset-8 bash

# Enter third party libraries directory
cd thirdparty

# Source third-party library versions, directory paths, compiler options
source thirdparty_versions.sh
source thirdparty_options.sh

# CentOS 7/RHEL7 ONLY:
# Install newer version of the OpenSSL
sudo yum install -y perl-core zlib-devel
cd openssl
tar --no-same-owner -xaf openssl-${SIODB_OPENSSL_VERSION}.tar.xz
cd openssl-${SIODB_OPENSSL_VERSION}
CFLAGS="${SIODB_TP_CFLAGS}" CXXFLAGS="${SIODB_TP_CXXFLAGS}" \
LDFLAGS="${SIODB_TP_LDFLAGS} -Wl,-rpath -Wl,${SIODB_OPENSSL_PREFIX}/lib" \
    ./config --prefix=${SIODB_OPENSSL_PREFIX} --openssldir=${SIODB_OPENSSL_PREFIX} shared zlib
make -j4
make -j4 test
sudo make -j4 install
cd ../..

# Install ANTLR4 executables
cd antlr4
sudo ./install.sh ${SIODB_ANTLR4_PREFIX}
cd ..

# Build and isntall ANTLR4 runtime library
cd antlr4-cpp-runtime
tar --no-same-owner -xaf antlr4-cpp-runtime-${SIODB_ANTLR4_CPP_RUNTIME_VERSION}-source.tar.xz
cd antlr4-cpp-runtime-${SIODB_ANTLR4_CPP_RUNTIME_VERSION}-source
mkdir build
cd build
CFLAGS="${SIODB_TP_CFLAGS}" CXXFLAGS="${SIODB_TP_CXXFLAGS}" LDFLAGS="${SIODB_TP_LDFLAGS}" \
    cmake -DCMAKE_INSTALL_PREFIX=${SIODB_ANTLR4_CPP_RUNTIME_PREFIX} -DCMAKE_BUILD_TYPE=ReleaseWithDebugInfo ..
make -j4
sudo make install
sudo ldconfig
cd ../../..

# Build and install date library
cd date
tar --no-same-owner -xaf date-${SIODB_LIBDATE_VERSION}.tar.xz
cd date-${SIODB_LIBDATE_VERSION}
mkdir build
cd build
CFLAGS="${SIODB_TP_CFLAGS}" CXXFLAGS="${SIODB_TP_CXXFLAGS}" LDFLAGS="${SIODB_TP_LDFLAGS}" \
    cmake -DCMAKE_INSTALL_PREFIX=${SIODB_LIBDATE_PREFIX} -DCMAKE_BUILD_TYPE=ReleaseWithDebugInfo \
          -DUSE_SYSTEM_TZ_DB=ON -DENABLE_DATE_TESTING=OFF -DBUILD_SHARED_LIBS=ON ..
make -j4
sudo make install
sudo ldconfig
cd ../../..

# Build and install Google Test library
cd googletest
tar --no-same-owner -xaf googletest-release-${SIODB_GTEST_VERSION}.tar.xz
cd googletest-release-${SIODB_GTEST_VERSION}/googlemock/scripts
./fuse_gmock_files.py include
sudo mkdir -p "${SIODB_GTEST_PREFIX}"
sudo cp -Rf include "${SIODB_GTEST_PREFIX}"
cd ../../../..

# Build and install JSON library
cd json
tar --no-same-owner -xaf json-${SIODB_JSON_VERSION}.tar.xz
cd json-${SIODB_JSON_VERSION}
mkdir build
cd build
CFLAGS="${SIODB_TP_CFLAGS}" CXXFLAGS="${SIODB_TP_CXXFLAGS}" LDFLAGS="${SIODB_TP_LDFLAGS}" \
    cmake -DCMAKE_INSTALL_PREFIX=${SIODB_JSON_PREFIX} -DCMAKE_BUILD_TYPE=ReleaseWithDebugInfo \
    -DJSON_BuildTests=OFF ..
make -j4
sudo make install
sudo ldconfig
cd ../../..

# Build and install Oat++ library
cd oatpp
tar --no-same-owner -xaf oatpp-${SIODB_OATPP_VERSION}.tar.xz
cd oatpp-${SIODB_OATPP_VERSION}
mkdir build
cd build
CFLAGS="${SIODB_TP_CFLAGS}" CXXFLAGS="${SIODB_TP_CXXFLAGS}" LDFLAGS="${SIODB_TP_LDFLAGS}" \
    cmake -DCMAKE_INSTALL_PREFIX=${SIODB_OATPP_PREFIX} -DCMAKE_BUILD_TYPE=ReleaseWithDebugInfo \
          -DBUILD_SHARED_LIBS=ON -DOATPP_BUILD_TESTS=OFF ..
make -j4
sudo make install
sudo ldconfig
cd ../../..

# Build and install Google Protobuf library
cd protobuf
tar --no-same-owner -xaf protobuf-all-${SIODB_PROTOBUF_VERSION}.tar.xz
cd protobuf-${SIODB_PROTOBUF_VERSION}
./autogen.sh
CFLAGS="${SIODB_TP_CFLAGS}" CXXFLAGS="${SIODB_TP_CXXFLAGS}" \
LDFLAGS="${SIODB_TP_LDFLAGS} -L${SIODB_PROTOBUF_PREFIX}/lib -Wl,-rpath -Wl,${SIODB_PROTOBUF_PREFIX}/lib" \
    ./configure "--prefix=${SIODB_PROTOBUF_PREFIX}" --enable-shared --enable-static
make -j4
sudo make install
sudo ldconfig

# Fix protoc if needed
${SIODB_PROTOBUF_PREFIX}/bin/protoc
if [[ $? != 0 ]]; then
    cd src
    cp -f protoc protoc.tmp
    sed -i "s+./.libs/libprotobuf.so ./.libs/libprotoc.so+-L${SIODB_PROTOBUF_PREFIX}/lib -lprotobuf -lprotoc -Wl,-rpath -Wl,${SIODB_PROTOBUF_PREFIX}/lib+g" protoc.tmp
    rm -f ./.libs/protoc ./.libs/lt-protoc
    ./protoc.tmp
    rm -f protoc.tmp
    cp -f ./.libs/lt-protoc ./.libs/protoc
    ldd ./.libs/protoc
    sudo install -t "${SIODB_PROTOBUF_PREFIX}/bin" ./.libs/protoc
    cd ..
fi

cd ../..

# Build and install Utf8cpp library
cd utf8cpp
tar --no-same-owner -xaf utfcpp-${SIODB_UTF8CPP_VERSION}.tar.xz
cd utfcpp-${SIODB_UTF8CPP_VERSION}
mkdir build
cd build
CFLAGS="${SIODB_TP_CFLAGS}" CXXFLAGS="${SIODB_TP_CXXFLAGS}" LDFLAGS="${SIODB_TP_LDFLAGS}" \
    cmake -DCMAKE_INSTALL_PREFIX=${SIODB_UTF8CPP_PREFIX} -DCMAKE_BUILD_TYPE=ReleaseWithDebugInfo \
          -DUTF8_TESTS=Off ..
make -j4
sudo make install
sudo ldconfig
cd ../../..

# Build and install xxHash library
cd xxHash
tar --no-same-owner -xaf xxHash-${SIODB_XXHASH_VERSION}.tar.xz
cd xxHash-${SIODB_XXHASH_VERSION}
mkdir build
cd build

# CentOS and RHEL
CFLAGS="${SIODB_TP_CFLAGS}" CXXFLAGS="${SIODB_TP_CXXFLAGS}" \
LDFLAGS="${SIODB_TP_LDFLAGS} -Wl,-rpath -Wl,${SIODB_XXHASH_PREFIX}/lib64" \
    cmake -DCMAKE_INSTALL_PREFIX=${SIODB_XXHASH_PREFIX} -DCMAKE_BUILD_TYPE=ReleaseWithDebugInfo \
          ../cmake_unofficial

# Ubuntu, Debian
CFLAGS="${SIODB_TP_CFLAGS}" CXXFLAGS="${SIODB_TP_CXXFLAGS}" \
LDFLAGS="${SIODB_TP_LDFLAGS} -Wl,-rpath -Wl,${SIODB_XXHASH_PREFIX}/lib" \
    cmake -DCMAKE_INSTALL_PREFIX=${SIODB_XXHASH_PREFIX} -DCMAKE_BUILD_TYPE=ReleaseWithDebugInfo \
          ../cmake_unofficial

sudo make -j4
sudo make install
sudo ldconfig
cd ../../..
```

### Go Libraries

Run following commands:

```bash
export GO_VERSION=1.15
/usr/local/go-${GO_VERSION}/bin/go get -u github.com/golang/protobuf/protoc-gen-go
/usr/local/go-${GO_VERSION}/bin/go get -u github.com/gin-gonic/gin
```

## System Setup (Debian, Ubuntu, CentOS, RHEL, SuSE, SLES)

One-time system setup commands (with explanation):

```bash
# Change to siodb repository root, CHANGE THIS to actua directory
cd siodb

# Add Siodb user and group
sudo useradd -s /sbin/nologin -d /var/lib/siodb siodb
sudo usermod -L siodb

# Create Siodb instance configuration directory
sudo mkdir -p /etc/siodb/instances
sudo chown -R siodb:siodb /etc/siodb
sudo chmod -R 0770 /etc/siodb

# Create default instance configuration
sudo -u siodb /etc/siodb/instances/siodb
sudo cp config/siodb.conf /etc/siodb/instances/siodb/config
sudo chmod 0600 /etc/siodb/instances/siodb/config
sudo chown siodb:siodb /etc/siodb/instances/siodb/config
sudo -u siodb dd if=/dev/random of=/etc/siodb/instances/siodb/master_key bs=16 count=1
sudo chmod 0600 /etc/siodb/instances/siodb/master_key
sudo cp config/sample_keys/rsa.pub /etc/siodb/instances/siodb/initial_access_key
sudo chmod 0600 /etc/siodb/instances/siodb/initial_access_key
sudo chown siodb:siodb /etc/siodb/instances/siodb/initial_access_key

# Create default data directory
sudo mkdir -p /var/lib/siodb
sudo chown -R siodb:siodb /var/lib/siodb
sudo chmod -R 0770 /var/lib/siodb

# Create default log directory
sudo mkdir -p /var/log/siodb
sudo chown -R siodb:siodb /var/log/siodb
sudo chmod -R 0770 /var/log/siodb

# Create UNIX socket directory
sudo mkdir -p /run/siodb/
sudo chown -R siodb:siodb /run/siodb
sudo chmod -R 0770 /run/siodb

# Make UNIX socket directory persistent across reboots
sudo /bin/sh -c 'echo "d /run/siodb 0770 siodb siodb -" >/usr/lib/tmpfiles.d/siodb.conf'

# Create lock file directory
sudo mkdir -p /run/lock/siodb/
sudo chown -R siodb:siodb /run/lock/siodb
sudo chmod -R 0770 /run/lock/siodb

# Make lock file directory persistent across reboots
sudo /bin/sh -c 'echo "d /run/lock/siodb 0770 siodb siodb -" >>/usr/lib/tmpfiles.d/siodb.conf'

# Adjust file descriptor limits for the user siodb
sudo /bin/sh -c 'echo "siodb hard nofile 524288" >>/etc/security/limits.conf'
sudo /bin/sh -c 'echo "siodb soft nofile 524288" >>/etc/security/limits.conf'
```

To allow running Siodb under your own user instead of user `siodb`:

```bash
# Add self to the group siodb
sudo usermod -a -G siodb `whoami`

# Adjust file descriptor limits
sudo /bin/sh -c 'echo "'`whoami`' hard nofile 524288" >>/etc/security/limits.conf'
sudo /bin/sh -c 'echo "'`whoami`' soft nofile 524288" >>/etc/security/limits.conf'
```

and re-login.

To allow running SQL tests under your own user (may be required on the CentOS and RHEL):

```bash
# 1. Performs above steps to allow running Siodb under your own user.

# 2. Adjust Siodb defult instance configuration file permissions
sudo chmod 0660 /etc/siodb/instances/siodb/config
sudo chmod 0660 /etc/siodb/instances/siodb/master_key
sudo chmod 0660 /etc/siodb/instances/siodb/initial_access_key

# 3. Edit default instance configuration file /etc/siodb/instances/siodb/config
#    set following parameter to "true"
allow_group_permissions_on_config_files = true
```

## Compiling Siodb

- Build debug version: `make -j4`. Build outputs will appear in the directory `debug/bin`.
- Build release version: `make -j4 DEBUG=0`. Build outputs will appear in the directory `release/bin`.
- List all available build commands: `make help`.

**NOTE:** Adjust `-jN` option in the above `make` commands according to available number of CPUs
  and memory on the your build host.

## Running Siodb

Before running Siodb, you need to create some instance configuration files:

- `/etc/siodb/instances/<configuration-name>/config` - instance configuration options file.
- `/etc/siodb/instances/<configuration-name>/master_key` - master encryption key for.
  Note that this file must be present even if master cipher ID is set to the `none`
  (in such case it can be just zero length file).

These files owner must be owned by the user `siodb` and group `siodb`. File mode must be `0400` or `0600`.
Debug build of Siodb also allows members of owner group to have access to these files.
There are some sample configurations in the directory `config` that may be used as starting point.

- Run Siodb server: `siodb --instance <configuration-name>`
- Run Siodb client in the admin mode: `siocli --admin <configuration-name>`
