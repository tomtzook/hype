
if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()
if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -DGNU_EFI_USE_MS_ABI -m64 -ffreestanding -Wall -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -Wall")

set(LDFLAGS "-nostdlib -shared -lgcc -Wl,-Bsymbolic -Wl,-nostdlib -Wl,-znocombreloc")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LDFLAGS}")

find_package(GNUefi REQUIRED)
include_directories(../arch/include)

set(HEADERS
        include/debug.h)

set(SOURCES
        src/main.cpp)

set(TARGET_NAME hype)

add_library(${TARGET_NAME} SHARED ${HEADERS} ${SOURCES})
target_compile_definitions(${TARGET_NAME} PUBLIC _DEBUG)
target_link_libraries(${TARGET_NAME} GnuEfi::GnuEfi ${GNUEFI_LIBRARIES} ${LDFLAGS})

add_custom_command(TARGET "${TARGET_NAME}" POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

function(efi_objcopy)
    cmake_parse_arguments(parsedargs "" "INFILE;OUTFILE" "SECTIONS" ${ARGN})
    add_custom_command(TARGET "${TARGET_NAME}" POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
            ${parsedargs_SECTIONS}
            --target=efi-app-x86_64
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
