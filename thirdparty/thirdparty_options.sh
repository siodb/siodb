# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

gcc_path=$(which gcc)

distro=$(lsb_release -is)
distro_version=$(lsb_release -rs)

if [[ "${distro}" == "Ubuntu" && "${distro_version}" == "18.04" ]]; then
    # Ubuntu 18.04 ONLY: Set compiler to gcc-9
    export CC=gcc-9
    export CXX=g++-9
    export LD=g++-9
else
    # All other systems (except Ubuntu 18.04)
    export CC=gcc
    export CXX=g++
    export LD=g++
fi

# All systems
export SIODB_TP_CFLAGS="-pipe -fexceptions -fasynchronous-unwind-tables \
    -fstack-clash-protection -fstack-protector-strong -grecord-gcc-switches \
    -fcf-protection=full -O2 -D_FORTIFY_SOURCE=2 -fPIC -g3 ${RHEL_DTS_CFLAGS}"

export SIODB_TP_CXXFLAGS="-pipe -fexceptions -fasynchronous-unwind-tables \
    -fstack-clash-protection -fstack-protector-strong -grecord-gcc-switches \
    -fcf-protection=full -O2 -D_FORTIFY_SOURCE=2 -fPIC -g3 ${RHEL_DTS_CXXFLAGS} \
    -D_GLIBCXX_ASSERTIONS"

export SIODB_TP_LDFLAGS="-Wl,-z,defs -Wl,-z,now -Wl,-z,relro -g3"
