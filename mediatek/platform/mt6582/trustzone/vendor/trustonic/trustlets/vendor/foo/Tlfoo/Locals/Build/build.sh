#!/bin/bash

t_sdk_root=$(readlink -f $(dirname $(readlink -f $0))/../../../../../../../../../../../trustzone/vendor/trustonic/t-base)

export COMP_PATH_ROOT=$(dirname $(readlink -f ${BASH_SOURCE[0]}))
export DRFOO_OUT_DIR=${TEE_DRIVER_OUTPUT_PATH}/drfoo
export DRFOO_DIR=${COMP_PATH_ROOT}/../../../Drfoo/Out

source ${t_sdk_root}/setup.sh

cd $(dirname $(readlink -f $0))
cd ../..

if [ ! -d Locals ]; then
  exit 1
fi

export TLSDK_DIR_SRC=${COMP_PATH_TlSdk}
export TLSDK_DIR=${COMP_PATH_TlSdk}
mkdir -p Out/Public
cp -f \
	Locals/Code/public/* \
	Out/Public/
echo "Running make..."	
make -f Locals/Code/makefile.mk "$@"
