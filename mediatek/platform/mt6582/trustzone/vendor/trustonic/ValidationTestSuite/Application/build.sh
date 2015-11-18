#!/bin/bash

if [ -z $COMP_PATH_ROOT ]; then
	echo "The build environment is not set!"
	echo "Trying to source setupDrivers.sh automatically!"
	source ../../setupDrivers.sh || exit 1
fi

ROOT_PATH=$(dirname $(readlink -f $BASH_SOURCE))

### 
###   Android NDK path
### 
NDK_BUILD=${COMP_PATH_AndroidNdkLinux}/ndk-build
PLATFORM=Generic
# Needs to change due to limitations in NDK build
export COMP_PATH_MobiCoreDriverLib=../../MobiCoreDriverLib/Out
export COMP_PATH_TlSampleRot13=$COMP_PATH_ROOT/TlSampleRot13/Out
export COMP_PATH_CppUTest=../CppUTest

   
# run NDK build
${NDK_BUILD} \
	-B \
	NDK_DEBUG=1 \
	NDK_PROJECT_PATH=$ROOT_PATH/Locals/Code \
	NDK_APPLICATION_MK=$ROOT_PATH/Locals/Code/Application.mk \
	NDK_MODULE_PATH=. \
	NDK_APP_OUT=$ROOT_PATH/Out/_build \
	APP_BUILD_SCRIPT=$ROOT_PATH/Locals/Code/Android.mk \
	APP_OPTIM=$MODE
