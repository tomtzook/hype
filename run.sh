#!/bin/bash

mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../scrips/toolchains/mingw-w64-x86_64.cmake
make
