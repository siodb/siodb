#!/bin/bash

set -x
set -e

version=4.9.2
this_script=$(realpath $0)
this_dir=$(dirname ${this_script})
prefix=/opt/siodb/lib/antlr-${version}

if [[ "$1" != "" ]]; then
    prefix="$1"
fi

install -d "${prefix}/bin"
install -d "${prefix}/lib"

tmp_dir=$(mktemp -d -t siodb-install-antlr-XXXXXXXXXX)
for f in grun4 antlr4; do
    cp -f "${this_dir}/${f}.in" "${tmp_dir}/${f}"
    sed -i "s+@@PREFIX@@+${prefix}+g" "${tmp_dir}/${f}"
    sed -i "s+@@VERSION@@+${version}+g" "${tmp_dir}/${f}"
    install -m 0755 -t "${prefix}/bin" "${tmp_dir}/${f}"
done
rm -rf "${tmp_dir}"

install -m 0644 -t "${prefix}/lib" "${this_dir}/antlr-${version}-complete.jar"
