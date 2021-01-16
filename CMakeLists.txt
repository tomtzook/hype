cmake_minimum_required(VERSION 3.16.3)
project(hypervisor C CXX ASM)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/scripts/modules/")

include(${CMAKE_SOURCE_DIR}/scripts/macros.cmake)
enable_asm()

#add_subdirectory(arch)
add_subdirectory(hype)
