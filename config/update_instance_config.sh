#!/bin/bash

set -e


if [[ $# -lt 1 ]]; then
    echo "Wrong number of parameters." 1>&2
    echo "Usage: $0 INSTANCE_NAME" 1>&2
    exit 1
fi

if [[ $EUID -ne 0 ]]; then
   echo "Only root can do this." 1>&2
   exit 100
fi

is_creating=0
instance_name=$1
instance_cfg_root=/etc/siodb/instances
instance_cfg_dir=${instance_cfg_root}/${instance_name}
config_file=${instance_name}.conf
master_key_file=${instance_name}.master_key

if [[ ! -d "${instance_cfg_dir}" ]]; then
    is_creating=1
    echo "Creating instance configuration dir ${instance_cfg_dir}"
    mkdir -p "${instance_cfg_dir}"
fi

echo "Copying config file"
cp -f ${config_file} ${instance_cfg_dir}/config
chown siodb:siodb ${instance_cfg_dir}/config
chmod u-x ${instance_cfg_dir}/config
chmod g-x ${instance_cfg_dir}/config
chmod o-rwx ${instance_cfg_dir}/config

echo "Copying system db key file"
if [[ -f ${master_key_file} ]]; then
    cp -f ${master_key_file} ${instance_cfg_dir}/master_key
else
    cp -f siodb.master_key ${instance_cfg_dir}/master_key
fi
chown siodb:siodb ${instance_cfg_dir}/master_key

if [[ ${is_creating} -eq 1 ]]; then
    echo "Instance configuration created."
else
    echo "Instance configuration updated."
fi
