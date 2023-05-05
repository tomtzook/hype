#!/bin/bash

if [[ -z "$1" ]]; then
  echo EDK2_PATH argument missing. Set it to point to your edk2 clone.
  exit 1
fi

EDK2_PATH="$1"

mkdir build_tmp
pushd build_tmp
cmake .. -DEDK2_PATH=${EDK2_PATH}
popd

cmake --build build_tmp --target edk2
rm -rf build_tmp
