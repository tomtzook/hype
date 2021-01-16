if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()
if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()

set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/bin/gcc)
set(CMAKE_LINKER /usr/bin/ld)
set(CMAKE_C_CREATE_SHARED_LIBRARY "<CMAKE_LINKER> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_CXX_CREATE_SHARED_LIBRARY "<CMAKE_LINKER> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

set(ARCH x86_64)
set(EFI_INCLUDE_PATH /usr/local/include/efi)
set(LIB_PATH /usr/local/lib)
set(EFI_LIB_PATH /usr/local/lib)
set(EFI_CRT_OBJS ${EFI_LIB_PATH}/crt0-efi-${ARCH}.o)
set(EFI_LDS ${EFI_LIB_PATH}/elf_${ARCH}_efi.lds)

list(APPEND COMPILE_FLAGS "-fno-stack-protector" "-fpic" "-fshort-wchar" "-mno-red-zone" "-Wall" "-DEFI_FUNCTION_WRAPPER")
set(LINK_FLAGS -nostdlib -T ${EFI_LDS} -shared -Bsymbolic -L ${EFI_LIB_PATH} -L ${LIB_PATH} ${EFI_CRT_OBJS})

include_directories(${EFI_INCLUDE_PATH} ${EFI_INCLUDE_PATH}/${ARCH} ${EFI_INCLUDE_PATH}/protocol)
#include_directories(../arch/include)

set(HEADERS
        include/debug.h)

set(SOURCES
        src/main.cpp)

set(TARGET_NAME hype)

add_library(${TARGET_NAME} SHARED ${HEADERS} ${SOURCES})
target_compile_definitions("${TARGET_NAME}" PUBLIC _DEBUG EFI_FUNCTION_WRAPPER)
target_compile_options(${TARGET_NAME} PRIVATE ${COMPILE_FLAGS})
target_link_options(${TARGET_NAME} PRIVATE ${LINK_FLAGS})
target_link_libraries(${TARGET_NAME} PRIVATE efi gnuefi)

add_custom_command(TARGET "${TARGET_NAME}" POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

function(efi_objcopy)
    cmake_parse_arguments(parsedargs "" "INFILE;OUTFILE" "SECTIONS" ${ARGN})
    add_custom_command(TARGET "${TARGET_NAME}" POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
            ${parsedargs_SECTIONS}
            --target=efi-app-x86_64 #  "efi-app", "efi-bsd", "efi-rtd"
            "${parsedargs_INFILE}"
            "${parsedargs_OUTFILE}")
endfunction()

set(ELF_SECTIONS
        -j .text
        -j .sdata
        -j .data
        -j .dynamic
        -j .dynsym
        -j .rel
        -j .rela
        -j .reloc)

efi_objcopy(INFILE "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${TARGET_NAME}.so"
        OUTFILE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}.efi"
        SECTIONS ${ELF_SECTIONS})

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(ELF_DEBUG_SECTIONS
            -j .debug_info
            -j .debug_abbrev
            -j .debug_loc
            -j .debug_aranges
            -j .debug_line
            -j .debug_macinfo
            -j .debug_str)

    efi_objcopy(INFILE "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${TARGET_NAME}.so"
            OUTFILE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}-debug.efi"
            SECTIONS ${ELF_SECTIONS} ${ELF_DEBUG_SECTIONS})
endif()
