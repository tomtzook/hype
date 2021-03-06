
list(APPEND COMPILE_FLAGS "-fno-stack-protector" "-fpic" "-fshort-wchar" "-mno-red-zone" "-Wall" "-DEFI_FUNCTION_WRAPPER")
set(LINK_FLAGS -nostdlib -T ${EFI_LDS} -shared -Bsymbolic -L ${EFI_LIB_PATH} -L ${LIB_PATH} ${EFI_CRT_OBJS})

include_directories(${EFI_INCLUDE_PATH} ${EFI_INCLUDE_PATH}/${ARCH} ${EFI_INCLUDE_PATH}/protocol)
include_directories(../common/include)
include_directories(../arch/include)
include_directories(include)

set(HEADERS
        include/commonefi.h
        include/hype.h include/common.h include/vcpu.h include/env.h include/vmcs.h)

set(SOURCES
        src/common/environment.cpp
        src/common/debug.cpp
        src/main.cpp
        src/efi.cpp
        src/hype.cpp src/vcpu.cpp src/env.cpp src/vmcs.cpp)

set(TARGET_NAME hype)

add_library(${TARGET_NAME} SHARED
        ${HEADERS}
        ${SOURCES}
        $<TARGET_OBJECTS:common_obj>
        $<TARGET_OBJECTS:arch_obj>
        )

target_compile_definitions("${TARGET_NAME}" PUBLIC _DEBUG EFI_FUNCTION_WRAPPER)
target_compile_options(${TARGET_NAME} PRIVATE ${COMPILE_FLAGS})
target_link_options(${TARGET_NAME} PRIVATE ${LINK_FLAGS})
target_link_libraries(${TARGET_NAME} PRIVATE efi gnuefi)

add_custom_command(TARGET "${TARGET_NAME}" POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
#--target=efi-app-x86_64 #  "efi-app", "efi-bsd", "efi-rtd"
function(efi_objcopy)
    cmake_parse_arguments(parsedargs "" "INFILE;OUTFILE" "SECTIONS" ${ARGN})
    add_custom_command(TARGET "${TARGET_NAME}" POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
            ${parsedargs_SECTIONS}
            --output-target=pei-x86-64
            --subsystem=efi-app     # efi-app  efi-rtd  efi-btd
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
