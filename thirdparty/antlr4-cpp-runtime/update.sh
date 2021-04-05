#!/usr/bin/env bash

set -e

if [[ $# == 0 ]]; then
    echo "Usage: $0 NEW_VERSION"
    exit 1
fi

_new_version=$1
_current_version=$(ls | grep antlr4-cpp-runtime | grep tar.xz | cut -f4 -d'-')

echo "Current version is ${_current_version}"

if [[ "$_current_version" == "$_new_version" ]]; then
    echo "Already up-to-date."
    exit 0
fi

# Downloads
if [[ -d antlr4 ]]; then rm -rf antlr4; fi
git clone https://github.com/antlr/antlr4
cd antlr4
git checkout ${_new_version}
cd runtime
_dir=antlr4-cpp-runtime-${_new_version}-source
mv -f Cpp ${_dir}
tar cvf ${_dir}.tar ${_dir}
xz -v9e ${_dir}.tar
mv ${_dir}.tar.xz ../..
cd ..
cp -f LICENSE.txt ../LICENSE-${_new_version}.txt
cd ..
rm -rf antlr4

# Remove old files
rm antlr4-cpp-runtime-${_current_version}-source.tar.xz
rm LICENSE-${_current_version}.txt

# git
git add .
git commit -m "Update ANTLR4 to version ${_new_version}"
