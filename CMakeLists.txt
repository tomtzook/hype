cmake_minimum_required(VERSION 3.16.3)
project(hypervisor C CXX ASM)

if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()
if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/scripts/modules/")

set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_CXX_COMPILER /usr/bin/clang)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_FLAGS "-fno-stack-protector -fshort-wchar -mno-red-zone -Wall --target x86_64-unknown-windows")
set(CMAKE_CXX_FLAGS "-fno-stack-protector -fshort-wchar -mno-red-zone -Wall -target x86_64-unknown-windows")

include(${CMAKE_SOURCE_DIR}/scripts/macros.cmake)
enable_asm()

#set(ARCH x86_64)
#set(EFI_INCLUDE_PATH /usr/local/include/efi)
#set(LIB_PATH /usr/local/lib)
#set(EFI_LIB_PATH /usr/local/lib)
#set(EFI_CRT_OBJS ${EFI_LIB_PATH}/crt0-efi-${ARCH}.o)
#set(EFI_LDS ${EFI_LIB_PATH}/elf_${ARCH}_efi.lds)

add_subdirectory(common)
add_subdirectory(arch)
add_subdirectory(hype)
