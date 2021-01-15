#!/bin/bash

mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../scripts/toolchains/mingw-w64-x86_64.cmake
make
