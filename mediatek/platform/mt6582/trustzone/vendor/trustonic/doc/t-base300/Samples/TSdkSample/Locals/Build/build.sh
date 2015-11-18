#!/bin/bash

# Source setup.sh
setup_script=""
search_dir=$(dirname $(readlink -f ${BASH_SOURCE[0]}))
cur_dir=${search_dir}

while [[ ${setup_script} == "" ]]
do
  setup_script=$( find $search_dir -name "setup.sh" )
  search_dir=$(dirname $search_dir)
done
if [[ ${setup_script} == "" ]]; then
  echo "ERROR: setup.sh not found"
  exit 1
fi
source $setup_script

if [[ -z "$COMP_PATH_OTA" ]] ;then
    echo "COMP_PATH_OTA is not set in setup.sh"
    exit 1
fi

if [[ -z "$COMP_PATH_Tools" ]] ;then
    echo "COMP_PATH_Tools is not set in setup.sh"
    exit 1
fi

cd ${cur_dir}/../Code

if [ "$MODE" == "Release" ]; then
  echo -e "Mode\t\t: Release"
  OPTIM=release
else
  echo -e "Mode\t\t: Debug"
  OPTIM=debug
fi

BUILD_TAG_FILE=src/com/trustonic/tsdk/sample/BuildTag.java
PACKAGE="com.trustonic.tsdk.sample"
BUILD_TAG=$(grep ComponentBuilder $BUILD_TAG_FILE)
if [ $? -eq 0 ] ; then
    VERSION_NAME=$(echo $BUILD_TAG | cut -d "-" -f1 | tr -d "*[:alpha:][:punct:] ")
    VERSION_NAME="ComponentBuilder $VERSION_NAME"
    VERSION_CODE=$(echo $BUILD_TAG | cut -d "-" -f2 | tr -d "*[:alpha:][:punct:] ")
else
    BUILD_TAG=$(grep BUILD_TAG $BUILD_TAG_FILE)
    VERSION_NAME=$(echo $BUILD_TAG | cut -d "," -f1 | tr -s " " "\n" | \
    grep \@)
    VERSION_CODE=$(echo $BUILD_TAG | cut -d "," -f2 | tr -d "*[:alpha:][:punct:] ")
fi

mkdir assets
if [ $TOOLCHAIN == "ARM" ]; then
cp -f ${COMP_PATH_TlAes}/Bin/$MODE/07020000000000000000000000000000.tlbin assets
cp -f ${COMP_PATH_TlRsa}/Bin/$MODE/07040000000000000000000000000000.tlbin assets
cp -f ${COMP_PATH_TlSampleRot13}/Bin/$MODE/04010000000000000000000000000000.tlbin assets
cp -f ${COMP_PATH_TlSampleSha256}/Bin/$MODE/07020000000000000000000000000000.tlbin assets
else
cp -f ${COMP_PATH_TlAes}/Bin/GNU/$MODE/07020000000000000000000000000000.tlbin assets
cp -f ${COMP_PATH_TlRsa}/Bin/GNU/$MODE/07040000000000000000000000000000.tlbin assets
cp -f ${COMP_PATH_TlSampleRot13}/Bin/GNU/$MODE/04010000000000000000000000000000.tlbin assets
cp -f ${COMP_PATH_TlSampleSha256}/Bin/GNU/$MODE/06010000000000000000000000000000.tlbin assets
fi
mkdir -p libs/armeabi
cp -f ${COMP_PATH_TlcAes}/Bin/$MODE/libTlcSampleAes.so             libs/armeabi
cp -f ${COMP_PATH_TlcSampleRot13}/Bin/$MODE/libTlcSampleRot13.so   libs/armeabi
cp -f ${COMP_PATH_TlcRsa}/Bin/$MODE/libTlcSampleRsa.so             libs/armeabi
cp -f ${COMP_PATH_TlcSampleSha256}/Bin/$MODE/libTlcSampleSha256.so libs/armeabi

cp -f ${COMP_PATH_OTA}/sppa-interface.jar              libs
if [ $? -ne 0 ]; then
    echo "ERROR: Could not find sppa-interface.jar library."
    exit 1
fi
cp -f ${COMP_PATH_OTA}/rootpa-interface*.jar           libs
if [ $? -ne 0 ]; then
    echo "ERROR: Could not find rootpa-interface.jar library."
    exit 1
fi
cp -f ${COMP_PATH_Tools}/scprov-jdk15on-1.47.0.2.jar   libs
if [ $? -ne 0 ]; then
    echo "ERROR: Could not find scprov-jdk15on-1.47.0.2.jar library. Please refer to Getting Started documentation to install it."
    exit 1
fi

cp -f ${COMP_PATH_Tools}/sc-light-jdk15on-1.47.0.2.jar libs
if [ $? -ne 0 ]; then
    echo "ERROR: Could not find sc-light-jdk15on-1.47.0.2.jar library. Please refer to Getting Started documentation to install it."
    exit 1
fi

ant $OPTIM

mkdir -p $cur_dir/../../Out/Bin/${MODE}
cp -f $cur_dir/../Code/bin/*.apk $cur_dir/../../Out/Bin/$MODE/