
include_directories(include)
set(HEADERS
        include/x86/cpuid.h
        include/x86/regs.h
        include/types.h
        include/x86/segment.h
        include/x86/intrinsic.h)

set(ASM_SOURCES
        src/x86/intrinsic/iregs.asm
        src/x86/intrinsic/iseg.asm)

set_source_files_properties(${ASM_SOURCES} PROPERTIES LANGUAGE ASM_NASM)

set(SOURCES
        src/x86/cpuid.cpp
        src/x86/segment.cpp
        src/x86/regs.cpp
        ${ASM_SOURCES})

add_library(arch_static STATIC ${HEADERS} ${SOURCES})
