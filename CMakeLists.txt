cmake_minimum_required(VERSION 3.17)
project(hypervisor CXX ASM)

set(CMAKE_CXX_STANDARD 14)

enable_language(ASM_NASM)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> \
    <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")

add_subdirectory(arch)
add_subdirectory(test)

