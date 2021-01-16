set(CMAKE_CXX_FLAGS "-fno-stack-protector -fpic -fshort-wchar -mno-red-zone -Wall")

include_directories(include)

set(HEADERS
        include/x86/cpuid.h
        include/x86/regs.h
        include/types.h
        include/x86/segment.h
        include/x86/intrinsic.h include/macros.h)

set(ASM_SOURCES
        src/x86/intrinsic/iregs.asm
        src/x86/intrinsic/iseg.asm)

set(SOURCES
        src/x86/cpuid.cpp
        src/x86/segment.cpp
        src/x86/regs.cpp
        ${ASM_SOURCES})

add_library(arch_static STATIC ${HEADERS} ${SOURCES})
target_compile_definitions(arch_static PUBLIC _DEBUG)
