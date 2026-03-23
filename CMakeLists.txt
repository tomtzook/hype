cmake_minimum_required(VERSION 3.16.3)
project(hypervisor C CXX ASM)

if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()
if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/scripts/modules/")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# /Oy-
# -target x86_64-pc-windows-msvc
# -mno-omit-leaf-frame-pointer -fno-omit-frame-pointer
set(CMAKE_C_FLAGS "-fno-rtti -fno-use-cxa-atexit -mno-omit-leaf-frame-pointer -fno-omit-frame-pointer -fno-threadsafe-statics -fno-stack-protector -fshort-wchar -mno-red-zone -Wall -target x86_64-pc-windows-msvc -D__x86_64__ -fno-exceptions -mstack-alignment=16 -g")
set(CMAKE_CXX_FLAGS "-fno-rtti -fno-use-cxa-atexit -mno-omit-leaf-frame-pointer -fno-omit-frame-pointer -fno-threadsafe-statics -fno-stack-protector -fshort-wchar -mno-red-zone -Wall -target x86_64-pc-windows-msvc -D__x86_64__ -fno-exceptions -mstack-alignment=16 -g")
set(CMAKE_CXX_LINK_FLAGS "-target x86_64-pc-windows-msvc -nostdlib -Wl,-entry:EfiMain -Wl,-subsystem:efi_application -fuse-ld=lld-link-19 -g")

include(${CMAKE_SOURCE_DIR}/scripts/macros.cmake)
enable_asm()

set(MINISTL_DEFINE_TYPES 1)
set(MINISTL_SIZET_SIZE 2)
set(MINISTL_DEFINE_CRT 1)
set(MINISTL_DEFINE_INTRINSIC 1)
set(PE_NO_STD 1)

add_subdirectory(arch)
add_subdirectory(edk)
add_subdirectory(framework)
add_subdirectory(environment)
add_subdirectory(gdbstub)
add_subdirectory(pe)
add_subdirectory(hype)
