cmake_minimum_required(VERSION 3.16.3)
project(hypervisor CXX ASM)

include(scrips/macros.cmake)

set(CMAKE_CXX_STANDARD 14)

# enable_language(ASM_NASM)
# set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -o <OBJECT> <SOURCE>")
# set(CMAKE_ASM_NASM_FLAGS_DEBUG "-g -Fdwarf")

enable_asm()


#enable_language(ASM_NASM)
# set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
#set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> \
#    <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")
#SET(CMAKE_ASM_FLAGS "${CFLAGS} -x assembler-with-cpp")

add_subdirectory(arch)
add_subdirectory(test)
