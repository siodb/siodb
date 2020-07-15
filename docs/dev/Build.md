# Building Siodb

- [Environment Preparation](#environment-preparation)
  - [Ubuntu 18.04 LTS](#ubuntu-1804-lts)
  - [Ubuntu 20.04 LTS](#ubuntu-2004-lts)
  - [Debian 10](#debian-10)
  - [CentOS 7](#centos-7)
  - [CentOS 8](#centos-8)
  - [RHEL 7](#rhel-7)
  - [RHEL 8](#rhel-8)
- [Building Third-Party Libraries](#building-third-party-libraries)
- [System Setup (Debian, Ubuntu, CentOS, RHEL, SuSE, SLES)](#system-setup-debian-ubuntu-centos-rhel-suse-sles)
- [Compiling Siodb](#compiling-siodb)
- [Running Siodb](#running-siodb)

## Environment Preparation

### Ubuntu 18.04 LTS

Run following commands:

```bash
cd $HOME

# Required tools and libraries
sudo apt install -y build-essential cmake doxygen gdb git graphviz gcc-8 g++-8 \
    libboost1.65-dev libboost-log1.65-dev libboost-program-options1.65-dev \
    libcurl4-openssl-dev libssl-dev lsb-release openjdk-11-jdk-headless python \
    pkg-config uuid-dev clang-format-10 ubuntu-dbgsym-keyring

# Set up alternatives for the clang-format
sudo update-alternatives --install /usr/bin/clang-format clang-format \
    /usr/lib/llvm-10/bin/clang-format 1
sudo update-alternatives --install /usr/bin/git-clang-format git-clang-format \
    /usr/lib/llvm-10/bin/git-clang-format 1
sudo update-alternatives --set clang-format /usr/lib/llvm-10/bin/clang-format
sudo update-alternatives --set git-clang-format \
    /usr/lib/llvm-10/bin/git-clang-format
```

Now, proceed to the section [Building Third-Party Libraries](#building-third-party-libraries).

### Ubuntu 20.04 LTS

Run following commands:

```bash
cd $HOME

# Required tools and libraries
sudo apt install -y build-essential cmake doxygen gdb git graphviz \
    libboost1.71-dev libboost-log1.71-dev libboost-program-options1.71-dev \
    libcurl4-openssl-dev libssl-dev lsb-release openjdk-11-jdk-headless \
    pkg-config python2 uuid-dev clang-format-10 ubuntu-dbgsym-keyring

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

Now, proceed to the section [Building Third-Party Libraries](#building-third-party-libraries).

### Debian 10

Run following commands:

```bash
cd $HOME

# Required tools and libraries
sudo apt install -y build-essential cmake doxygen gdb git graphviz \
    libboost1.67-dev libboost-log1.67-dev libboost-program-options1.67-dev \
    libcurl4-openssl-dev libssl-dev lsb-release openjdk-11-jdk-headless \
    pkg-config python2 uuid-dev wget

# Install clang-9. This one is for SLES, but works on the Debian 10 too.
sudo apt install -y libncurses5
mkdir /tmp/getllvm
cd /tmp/getllvm
wget http://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-linux-sles11.3.tar.xz
tar xaf clang+llvm-9.0.0-x86_64-linux-sles11.3.tar.xz
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

Now, proceed to the section [Building Third-Party Libraries](#building-third-party-libraries).

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
sudo yum install -y boost169-devel cmake3 curl devtoolset-8-toolchain doxygen \
    gcc gcc-c++ java-1.8.0-openjdk-headless libatomic libcurl-devel libuuid-devel \
    openssl-devel pkgconfig python redhat-lsb uuid-devel wget zlib-devel

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

# Install clang-9. This one is for SLES, but works on the CentOS 7 too.
mkdir /tmp/getllvm
cd /tmp/getllvm
wget http://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-linux-sles11.3.tar.xz
tar xaf clang+llvm-9.0.0-x86_64-linux-sles11.3.tar.xz
mv -f clang+llvm-9.0.0-x86_64-linux-sles11.3  local
sudo cp -Rf local /usr
sudo ldconfig
sudo mandb
cd $HOME
rm -rf /tmp/getllvm

# Permanently tell ldconfig to scan /usr/local/lib when updating cache
sudo /bin/sh -c 'echo "/usr/local/lib" >/etc/ld.so.conf.d/usr-local-lib.conf'
sudo ldconfig

# Create links for the libatomic - required by Oat++
sudo ln -s /usr/lib/libatomic.so.1.0.0 /usr/lib/libatomic.so
sudo ln -s /usr/lib64/libatomic.so.1.0.0 /usr/lib64/libatomic.so
```

Now, proceed to the section [Building Third-Party Libraries](#building-third-party-libraries).

### CentOS 8

Run following commands:

```bash
cd $HOME

# Enable additional repositories
sudo yum install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm

# Update your system
sudo yum -y update

# Install required tools and libraries
sudo yum install -y boost-devel cmake curl gcc gcc-c++ java-1.8.0-openjdk-headless \
    libatomic libcurl-devel libuuid-devel openssl-devel python2 pkgconfig \
    redhat-lsb wget zlib-devel

# Install clang-9. This one is for SLES, but works on the CentOS 8 too.
mkdir /tmp/getllvm
cd /tmp/getllvm
wget http://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-linux-sles11.3.tar.xz
tar xaf clang+llvm-9.0.0-x86_64-linux-sles11.3.tar.xz
mv -f clang+llvm-9.0.0-x86_64-linux-sles11.3  local
sudo cp -Rf local /usr
sudo ldconfig
sudo mandb
cd $HOME
rm -rf /tmp/getllvm

# Permanently tell ldconfig to scan /usr/local/lib when updating cache
sudo /bin/sh -c 'echo "/usr/local/lib" >/etc/ld.so.conf.d/usr-local-lib.conf'
sudo ldconfig

# Link Python 2 (required by Google Test fuse script)
sudo ln -s /usr/bin/python2 /usr/bin/python
```

Now, proceed to the section [Building Third-Party Libraries](#building-third-party-libraries).

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
sudo yum install -y boost169-devel cmake3 curl devtoolset-8-toolchain doxygen \
    gcc gcc-c++ java-1.8.0-openjdk-headless libatomic libcurl-devel libuuid-devel \
    openssl-devel pkgconfig python redhat-lsb uuid-devel wget zlib-devel

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

# Install clang-9. This one is for SLES, but works on the CentOS 7 too.
mkdir /tmp/getllvm
cd /tmp/getllvm
wget http://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-linux-sles11.3.tar.xz
tar xaf clang+llvm-9.0.0-x86_64-linux-sles11.3.tar.xz
mv -f clang+llvm-9.0.0-x86_64-linux-sles11.3  local
sudo cp -Rf local /usr
sudo ldconfig
sudo mandb
cd $HOME
rm -rf /tmp/getllvm

# Permanently tell ldconfig to scan /usr/local/lib when updating cache
sudo /bin/sh -c 'echo "/usr/local/lib" >/etc/ld.so.conf.d/usr-local-lib.conf'
sudo ldconfig

# Create links for the libatomic - required by Oat++
sudo ln -s /usr/lib/libatomic.so.1.0.0 /usr/lib/libatomic.so
sudo ln -s /usr/lib64/libatomic.so.1.0.0 /usr/lib64/libatomic.so
```

Now, proceed to the section [Building Third-Party Libraries](#building-third-party-libraries).

### RHEL 8

Run following commands:

```bash
cd $HOME

# Enable additional repositories
sudo yum install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm

# Update your system
sudo yum -y update

# Install required tools and libraries
sudo yum install -y boost-devel cmake curl doxygen gcc gcc-c++ \
    java-1.8.0-openjdk-headless libatomic libcurl-devel libuuid-devel \
    openssl-devel pkgconfig python2 redhat-lsb wget zlib-devel

# Install clang-9. This one is for SLES, but works on the CentOS 8 too.
mkdir /tmp/getllvm
cd /tmp/getllvm
wget http://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-linux-sles11.3.tar.xz
tar xaf clang+llvm-9.0.0-x86_64-linux-sles11.3.tar.xz
mv -f clang+llvm-9.0.0-x86_64-linux-sles11.3  local
sudo cp -Rf local /usr
sudo ldconfig
sudo mandb
cd $HOME
rm -rf /tmp/getllvm

# Permanently tell ldconfig to scan /usr/local/lib when updating cache
sudo /bin/sh -c 'echo "/usr/local/lib" >/etc/ld.so.conf.d/usr-local-lib.conf'
sudo ldconfig

# Link python 2
sudo ln -s /usr/bin/python2 /usr/bin/python
```

Now, proceed to the section [Building Third-Party Libraries](#building-third-party-libraries).

## Building Third-Party Libraries

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
cp -fv tools/git_hooks/siodb-clang-format.hook .git/hooks/pre-commit

# CentOS 7/RHEL 7 ONLY: Enable devtoolset-8
scl enable devtoolset-8 bash
export RHEL_DTS8_CFLAGS=-mcet
export RHEL_DTS8_CXXFLAGS=-mcet

# Ubuntu 18.04 ONLY: Set compiler to gcc-8
export CC=gcc-8
export CXX=g++-8
export LD=g++-8

# All other sustems (except Ubuntu 18.04)
export CC=gcc
export CXX=g++
export LD=g++

# All systems
export SIODB_TP_CFLAGS="-pipe -fexceptions -fasynchronous-unwind-tables \
    -fstack-clash-protection -fstack-protector-strong -grecord-gcc-switches \
    -fcf-protection=full -O2 -D_FORTIFY_SOURCE=2 -fPIC -g3 ${RHEL_DTS8_CFLAGS}"
export SIODB_TP_CXXFLAGS="-pipe -fexceptions -fasynchronous-unwind-tables \
    -fstack-clash-protection -fstack-protector-strong -grecord-gcc-switches \
    -fcf-protection=full -O2 -D_FORTIFY_SOURCE=2 -fPIC -g3 ${RHEL_DTS8_CXXFLAGS} \
    -D_GLIBCXX_ASSERTIONS"
export SIODB_TP_LDFLAGS="-Wl,-z,defs -Wl,-z,now -Wl,-z,relro -g3"
export SIODB_TP_ROOT=/opt/siodb/lib

# Enter third party libraries directory
cd thirdparty

# CentOS 7/RHEL7 ONLY:
# Install newer version of the OpenSSL
export OPENSSL_VERSION=1.1.1g
export OPENSSL_PREFIX=${SIODB_TP_ROOT}/openssl-${OPENSSL_VERSION}
sudo yum install -y perl-core libtemplate-perl zlib-devel
cd openssl
tar xaf openssl-${OPENSSL_VERSION}.tar.xz
cd openssl-${OPENSSL_VERSION}
CFLAGS="${SIODB_TP_CFLAGS}" CXXFLAGS="${SIODB_TP_CXXFLAGS}" LDFLAGS="${SIODB_TP_LDFLAGS}" \
    ./config --prefix=${OPENSSL_PREFIX} --openssldir=${OPENSSL_PREFIX} shared zlib
make -j4
make -j4 test
sudo make -j4 install
cd ../..

# Install ANTLR4 executables
export ANTLR4_VERSION=4.8
export ANTLR4_PREFIX=${SIODB_TP_ROOT}/antlr-${ANTLR4_VERSION}
cd antlr4
sudo ./install.sh ${ANTLR4_PREFIX}
cd ..

# Build and isntall ANTLR4 runtime library
export ANTLR4_CPP_RUNTIME_VERSION=4.8
export ANTLR4_CPP_RUNTIME_PREFIX=${SIODB_TP_ROOT}/antlr4-cpp-runtime-${ANTLR4_CPP_RUNTIME_VERSION}
cd antlr4-cpp-runtime
tar xaf antlr4-cpp-runtime-${ANTLR4_CPP_RUNTIME_VERSION}-source.tar.xz
cd antlr4-cpp-runtime-${ANTLR4_CPP_RUNTIME_VERSION}-source
mkdir build
cd build
CFLAGS="${SIODB_TP_CFLAGS}" CXXFLAGS="${SIODB_TP_CXXFLAGS}" LDFLAGS="${SIODB_TP_LDFLAGS}" \
    cmake -DCMAKE_INSTALL_PREFIX=${ANTLR4_CPP_RUNTIME_PREFIX} -DCMAKE_BUILD_TYPE=ReleaseWithDebugInfo ..
make -j4
sudo make install
sudo ldconfig
cd ../../..

# Build and install date library
export LIBDATE_VERSION=20190911
export LIBDATE_PREFIX=${SIODB_TP_ROOT}/date-${LIBDATE_VERSION}
cd date
tar xaf date-${LIBDATE_VERSION}.tar.xz
cd date-${LIBDATE_VERSION}
mkdir build
cd build
CFLAGS="${SIODB_TP_CFLAGS}" CXXFLAGS="${SIODB_TP_CXXFLAGS}" LDFLAGS="${SIODB_TP_LDFLAGS}" \
    cmake -DCMAKE_INSTALL_PREFIX=${LIBDATE_PREFIX} -DCMAKE_BUILD_TYPE=ReleaseWithDebugInfo \
          -DUSE_SYSTEM_TZ_DB=ON -DENABLE_DATE_TESTING=OFF -DBUILD_SHARED_LIBS=ON ..
make -j4
sudo make install
sudo ldconfig
cd ../../..

# Build and install Google Test library
export GTEST_VERSION=1.8.1
export GTEST_PREFIX=${SIODB_TP_ROOT}/gtest-gmock-${GTEST_VERSION}
cd googletest
tar xaf googletest-release-${GTEST_VERSION}.tar.xz
cd googletest-release-${GTEST_VERSION}/googlemock/scripts
./fuse_gmock_files.py include
sudo mkdir -p "${GTEST_PREFIX}"
sudo cp -Rf include "${GTEST_PREFIX}"
cd ../../../..

# Build and install Oat++ library
export OATPP_VERSION=1.1.0
export OATPP_PREFIX=${SIODB_TP_ROOT}/oatpp-${OATPP_VERSION}
cd oatpp
tar xaf oatpp-${OATPP_VERSION}.tar.xz
cd oatpp-${OATPP_VERSION}
mkdir build
cd build
CFLAGS="${SIODB_TP_CFLAGS}" CXXFLAGS="${SIODB_TP_CXXFLAGS}" LDFLAGS="${SIODB_TP_LDFLAGS}" \
    cmake -DCMAKE_INSTALL_PREFIX=${OATPP_PREFIX} -DCMAKE_BUILD_TYPE=ReleaseWithDebugInfo \
          -DBUILD_SHARED_LIBS=ON -DOATPP_BUILD_TESTS=OFF ..
make -j4
sudo make install
sudo ldconfig
cd ../../..

# Build and install Google Protobuf library
export PROTOBUF_VERSION=3.11.4
export PROTOBUF_PREFIX=${SIODB_TP_ROOT}/protobuf-${PROTOBUF_VERSION}
cd protobuf
tar xaf protobuf-all-${PROTOBUF_VERSION}.tar.xz
cd protobuf-${PROTOBUF_VERSION}
CFLAGS="${SIODB_TP_CFLAGS}" CXXFLAGS="${SIODB_TP_CXXFLAGS}" \
LDFLAGS="${SIODB_TP_LDFLAGS} -L${PROTOBUF_PREFIX}/lib -Wl,-rpath -Wl,${PROTOBUF_PREFIX}/lib" \
    ./configure "--prefix=${PROTOBUF_PREFIX}"
make -j4
sudo make install
sudo ldconfig

# Sometimes protoc somehow gets linked against wrong libraries
# and we have to fix that manually: patch protoc wrapper script and force
# relinking of the protoc executable with correct libraries
export PROTOC_CORRECT_LIB_COUNT=$(ldd "${PROTOBUF_PREFIX}/bin/protoc" | grep ${PROTOBUF_PREFIX}/lib | wc -l
)
echo "PROTOC_CORRECT_LIB_COUNT=${PROTOC_CORRECT_LIB_COUNT}"
if [[ PROTOC_CORRECT_LIB_COUNT != 2 ]]; then
    cd src
    cp -f protoc protoc.tmp
    sed -i "s+./.libs/libprotobuf.so ./.libs/libprotoc.so+-L${PROTOBUF_PREFIX}/lib -lprotobuf -lprotoc -Wl,-rpath -Wl,${PROTOBUF_PREFIX}/lib+g" protoc.tmp
    rm -f ./.libs/protoc ./.libs/lt-protoc
    ./protoc.tmp
    rm -f protoc.tmp
    cp -f ./.libs/lt-protoc ./.libs/protoc
    ldd ./.libs/protoc
    sudo install -t "${PROTOBUF_PREFIX}/bin" ./.libs/protoc
    cd ..
fi

cd ../..

# Build and install Utf8cpp library
export UTF8CPP_VERSION=3.1
export UTF8CPP_PREFIX=${SIODB_TP_ROOT}/utf8cpp-${UTF8CPP_VERSION}
cd utf8cpp
tar xaf utfcpp-${UTF8CPP_VERSION}.tar.xz
cd utfcpp-${UTF8CPP_VERSION}
mkdir build
cd build
CFLAGS="${SIODB_TP_CFLAGS}" CXXFLAGS="${SIODB_TP_CXXFLAGS}" LDFLAGS="${SIODB_TP_LDFLAGS}" \
    cmake -DCMAKE_INSTALL_PREFIX=${UTF8CPP_PREFIX} -DCMAKE_BUILD_TYPE=ReleaseWithDebugInfo \
          -DUTF8_TESTS=Off ..
make -j4
sudo make install
sudo ldconfig
cd ../../..

# Build and install xxHash library
export XXHASH_VERSION=0.7.2
export XXHASH_PREFIX=${SIODB_TP_ROOT}/xxHash-${XXHASH_VERSION}
cd xxHash
tar xaf xxHash-${XXHASH_VERSION}.tar.xz
cd xxHash-${XXHASH_VERSION}
mkdir build
cd build
CFLAGS="${SIODB_TP_CFLAGS}" CXXFLAGS="${SIODB_TP_CXXFLAGS}" LDFLAGS="${SIODB_TP_LDFLAGS}" \
    cmake -DCMAKE_INSTALL_PREFIX=${XXHASH_PREFIX} -DCMAKE_BUILD_TYPE=ReleaseWithDebugInfo \
          ../cmake_unofficial
sudo make -j4
sudo make install
sudo ldconfig
cd ../../..

# Build and install message compiler
cd ../tools/message_compiler
make -j4
sudo make install
cd ../..
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
sudo -u siodb dd if=/dev/urandom of=/etc/siodb/instances/siodb/system_db_key bs=16 count=1
sudo chmod 0600 /etc/siodb/instances/siodb/system_db_key
sudo cp config/sample_keys/rsa /etc/siodb/instances/siodb/initial_access_key
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
sudo chmod 0660 /etc/siodb/instances/siodb/system_db_key
sudo chmod 0660 /etc/siodb/instances/siodb/initial_access_key

# 3. Edit default instance configuration file /etc/siodb/instances/siodb/config
#    set following parameter to "true"
allow_group_permissions_on_config_files = true
```

## Compiling Siodb

- Build debug version: `make -j4`. Build outputs will appear in the directory `debug/bin`.
- Build release version: `make -j4 DEBUG=0`. Build outputs will appear in the directory `release/bin`.
- List all available build commands: `make help`.

**NOTE:** Adjust `-jN` option in the above `make` commands according to available number of CPUs and memory
  on the build host.

## Running Siodb

Before running Siodb, you need to create some instance configuration files:

- `/etc/siodb/instances/<configuration-name>/config` - instance configuration options file.
- `/etc/siodb/instances/<configuration-name>/system_db_key` - encryption key for the system database.
  Note that this file must be present even if encryption of the system database is set to the `none`
  (in such case it can be just zero length).

These files owner must be owned by the user `siodb` and group `siodb`. File mode must be `0400` or `0600`.
Debug build of Siodb also allows members of owner group to have access to these files.
There are some sample configurations in the directory `config` that may be used as starting point.

- Run Siodb server: `siodb --instance <configuration-name>`
- Run Siodb client in the admin mode: `siocli --admin <configuration-name>`
