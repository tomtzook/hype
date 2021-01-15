cmake_minimum_required(VERSION 3.16.3)
project(hypervisor CXX ASM)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/scripts/modules/")

include(${CMAKE_SOURCE_DIR}/scripts/macros.cmake)
enable_asm()

# enable_language(ASM_NASM)
# set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -o <OBJECT> <SOURCE>")
# set(CMAKE_ASM_NASM_FLAGS_DEBUG "-g -Fdwarf")
#enable_language(ASM_NASM)
# set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
#set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> \
#    <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")
#SET(CMAKE_ASM_FLAGS "${CFLAGS} -x assembler-with-cpp")

add_subdirectory(arch)
add_subdirectory(test)
