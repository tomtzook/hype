#!/bin/bash

if [[ -z "$1" ]]; then
  echo EDK2_PATH argument missing. Set it to point to your edk2 clone.
  exit 1
fi

EDK2_PATH="$1"
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

echo Copying package to edk2 root
cp -r ${SCRIPT_DIR}/edk/HypePkg ${EDK2_PATH}/

if [ ! -f ${EDK2_PATH}/Conf/target.txt ]; then
    echo Creating backup of original Conf/target.txt
    cp ${EDK2_PATH}/Conf/target.txt ${EDK2_PATH}/Conf/target.txt.old
fi

echo Copying target definitions to edk2 root
cp ${SCRIPT_DIR}/edk/target.txt ${EDK2_PATH}/Conf/target.txt

echo Running build for HypePkg

pushd ${EDK2_PATH}
. ./edksetup.sh BaseTools
build
popd

echo Copying target definitions to edk2 root
cp ${SCRIPT_DIR}/edk/target_ovmf.txt ${EDK2_PATH}/Conf/target.txt

echo Running build for OvmfPkg

pushd ${EDK2_PATH}
. ./edksetup.sh BaseTools
build
popd


echo Restoring edk2 root

if [ ! -f ${EDK2_PATH}/Conf/target.txt.old ]; then
  mv ${EDK2_PATH}/Conf/target.txt.old ${EDK2_PATH}/Conf/target.txt
fi

rm -r ${EDK2_PATH}/HypePkg

echo Copying build data
BUILD_ROOT=${EDK2_PATH}/Build/HypeX64/RELEASE_CLANGPDB/X64
BUILD_ROOT_OVMF=${EDK2_PATH}/Build/OvmfX64/RELEASE_GCC5
DST_ROOT=${SCRIPT_DIR}/.edk2build

rm -rf ${DST_ROOT}

mkdir -p ${DST_ROOT}/libs
pushd ${BUILD_ROOT}
find . -name \*.lib -exec cp {} ${DST_ROOT}/libs \;
popd

rm ${DST_ROOT}/libs/Hype.lib

cp ${BUILD_ROOT}/HypePkg/Hype/Hype/DEBUG/AutoGen.c ${DST_ROOT}

cp ${BUILD_ROOT_OVMF}/FV/OVMF.fd ${DST_ROOT}

echo Done