# Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Generate version variables from the $(ROOT)/mk/ThirdpartyLibVersions.mk

_tmpfile=$(mktemp)

_gen_vars() {
  while read -r _line; do
    local name=$(echo "${_line}" | cut -f1 -d':')
    local value=$(echo "${_line}" | cut -f2 -d'=')
    echo "${name}: ${value}"
    echo "export SIODB_${name}=${value}" >> "${_tmpfile}"
  done
}

# https://stackoverflow.com/a/246128/1540501
_SOURCE="${BASH_SOURCE[0]}"
while [ -h "$_SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  _DIR="$( cd -P "$( dirname "$_SOURCE" )" >/dev/null 2>&1 && pwd )"
  _SOURCE="$(readlink "$_SOURCE")"
  # if $_SOURCE was a relative symlink, we need to resolve it relative to the path
  # where the symlink file was located
  [[ $S_OURCE != /* ]] && _SOURCE="$_DIR/$_SOURCE" 
done
_DIR="$( cd -P "$( dirname "$_SOURCE" )" >/dev/null 2>&1 && pwd )"

_mk=$(realpath "${_DIR}/../mk/ThirdpartyLibVersions.mk")
echo $_mk
_sl=$(grep -n BEGIN_THIRDPARTY_LIB_VERSIONS "${_mk}" | cut -f1 -d':')
_sl=$((${_sl} + 1))
_el=$(grep -n END_THIRDPARTY_LIB_VERSIONS "${_mk}" | cut -f1 -d':')
_nl=$((${_el} - ${_sl}))
tail -n+${_sl} ${_mk} | head -n${_nl} | _gen_vars
# cat "${_tmpfile}"
source "${_tmpfile}"
rm -f "${_tmpfile}"

# Prefixes
export SIODB_TP_ROOT=/opt/siodb/lib
export SIODB_ANTLR4_PREFIX=${SIODB_TP_ROOT}/antlr-${SIODB_ANTLR4_VERSION}
export SIODB_ANTLR4_CPP_RUNTIME_PREFIX=${SIODB_TP_ROOT}/antlr4-cpp-runtime-${SIODB_ANTLR4_CPP_RUNTIME_VERSION}
export SIODB_LIBDATE_PREFIX=${SIODB_TP_ROOT}/date-${SIODB_LIBDATE_VERSION}
export SIODB_GTEST_PREFIX=${SIODB_TP_ROOT}/googletest-${SIODB_GTEST_VERSION}
export SIODB_JSON_PREFIX=${SIODB_TP_ROOT}/json-${SIODB_JSON_VERSION}
export SIODB_OPENSSL_PREFIX=${SIODB_TP_ROOT}/openssl-${SIODB_OPENSSL_VERSION}
export SIODB_PROTOBUF_PREFIX=${SIODB_TP_ROOT}/protobuf-${SIODB_PROTOBUF_VERSION}
export SIODB_UTF8CPP_PREFIX=${SIODB_TP_ROOT}/utf8cpp-${SIODB_UTF8CPP_VERSION}
export SIODB_XXHASH_PREFIX=${SIODB_TP_ROOT}/xxHash-${SIODB_XXHASH_VERSION}
