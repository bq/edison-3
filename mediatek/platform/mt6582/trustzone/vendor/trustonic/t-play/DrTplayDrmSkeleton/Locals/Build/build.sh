#!/bin/bash
cd $(dirname $(readlink -f $0)) 
export COMP_PATH_ROOT=../..
source ${COMP_PATH_ROOT}/../setup.sh 
export DRSDK_DIR_SRC=${COMP_PATH_ROOT}/${COMP_PATH_DrSdk} 
export TLSDK_DIR_SRC=${COMP_PATH_ROOT}/${COMP_PATH_TlSdk} 
echo "Running make..."
make -f ../Code/makefile.mk "$@"
