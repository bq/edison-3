#!/bin/bash
t_sdk_root=$(readlink -f $(dirname $(readlink -f $0))/../../../../../../../../../../../trustzone/vendor/trustonic/t-base)

export COMP_PATH_ROOT=$(dirname $(readlink -f ${BASH_SOURCE[0]}))
export COMP_PATH_Tlfoo=${COMP_PATH_ROOT}/../../../Tlfoo/Out
TLC_NAME=tlcfoo

if [ "$TEE_MODE" == "Release" ]; then
  echo -e "Mode\t\t: Release"
  OUT_DIR=${TEE_TLC_OUTPUT_PATH}/${TLC_NAME}/Release
  OPTIM=release
else
  echo -e "Mode\t\t: Debug"
  OUT_DIR=${TEE_TLC_OUTPUT_PATH}/${TLC_NAME}/Debug
  OPTIM=debug
fi

source ${t_sdk_root}/setup.sh

# go to project root
cd $(dirname $(readlink -f $0))/../..

### ---------------- Generic Build Command ----------------

# run NDK build
${NDK_BUILD} \
    -B \
    NDK_DEBUG=1 \
    NDK_PROJECT_PATH=Locals/Code \
    NDK_APPLICATION_MK=Locals/Code/Application.mk \
    NDK_MODULE_PATH=${t_sdk_root} \
    NDK_APP_OUT=Out/_build \
    APP_BUILD_SCRIPT=Locals/Code/Android.mk \
    APP_OPTIM=$OPTIM

mkdir -p $OUT_DIR

cp -r $PWD/Out/_build/local/armeabi/* $OUT_DIR

echo
echo Output directory of build is $OUT_DIR

