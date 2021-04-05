#!/usr/bin/env bash

set -e

if [[ $# == 0 ]]; then
    echo "Usage: $0 NEW_VERSION"
    exit 1
fi

_new_version=$1
_current_version=$(grep version= install.sh | cut -f2 -d'=')

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
cp -f LICENSE.txt ../LICENSE-${_new_version}.txt
cd ..
rm -rf antlr4

wget https://www.antlr.org/download/antlr-${_new_version}-complete.jar

# Transformations
sed -i "s+version=${_current_version}+version=${_new_version}+g" install.sh

# Remove old files
rm antlr-${_current_version}-complete.jar
rm LICENSE-${_current_version}.txt

# git
git add .
git commit -m "Update ANTLR4 to version ${_new_version}"
