gcc_path=$(which gcc)
if [[ "${gcc_path}" == "/opt/rh/devtoolset-8/root/usr/bin/gcc" ]]; then
    export RHEL_DTS8_CFLAGS=-mcet
    export RHEL_DTS8_CXXFLAGS=-mcet
fi

distro=$(lsb_release -is)
distro_version=$(lsb_release -rs)

if [[ "${distro}" == "Ubuntu" && "${distro_version}" == "18.04" ]]; then
    # Ubuntu 18.04 ONLY: Set compiler to gcc-8
    export CC=gcc-8
    export CXX=g++-8
    export LD=g++-8
else
    # All other systems (except Ubuntu 18.04)
    export CC=gcc
    export CXX=g++
    export LD=g++
fi

# All systems
export SIODB_TP_CFLAGS="-pipe -fexceptions -fasynchronous-unwind-tables \
    -fstack-clash-protection -fstack-protector-strong -grecord-gcc-switches \
    -fcf-protection=full -O2 -D_FORTIFY_SOURCE=2 -fPIC -g3 ${RHEL_DTS8_CFLAGS}"
export SIODB_TP_CXXFLAGS="-pipe -fexceptions -fasynchronous-unwind-tables \
    -fstack-clash-protection -fstack-protector-strong -grecord-gcc-switches \
    -fcf-protection=full -O2 -D_FORTIFY_SOURCE=2 -fPIC -g3 ${RHEL_DTS8_CXXFLAGS} \
    -D_GLIBCXX_ASSERTIONS"
export SIODB_TP_LDFLAGS="-Wl,-z,defs -Wl,-z,now -Wl,-z,relro -g3"
