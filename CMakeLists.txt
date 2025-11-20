cmake_minimum_required(VERSION 3.16.3)
project(hypervisor C CXX ASM)

if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()
if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/scripts/modules/")

set(CMAKE_C_COMPILER /usr/lib/llvm-18/bin/clang)
set(CMAKE_CXX_COMPILER /usr/lib/llvm-18/bin/clang++)
set(CMAKE_CXX_LINK_EXECUTABLE "/usr/lib/llvm-18/bin/clang++ <CMAKE_CXX_LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_C_FLAGS "-fno-stack-protector -fshort-wchar -mno-red-zone -Wall -target x86_64-pc-windows-msvc -D__x86_64__ -fno-exceptions -mstack-alignment=16")
set(CMAKE_CXX_FLAGS "-fno-stack-protector -fshort-wchar -mno-red-zone -Wall -target x86_64-pc-windows-msvc -D__x86_64__ -fno-exceptions -mstack-alignment=16")
set(CMAKE_CXX_LINK_FLAGS "-target x86_64-pc-windows-msvc -nostdlib -Wl,-entry:EfiMain -Wl,-subsystem:efi_application -fuse-ld=lld-link-18 -v")

include(${CMAKE_SOURCE_DIR}/scripts/macros.cmake)
enable_asm()

add_subdirectory(arch)
add_subdirectory(edk)
add_subdirectory(framework)
add_subdirectory(hype)
