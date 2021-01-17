set(CMAKE_CXX_FLAGS "-fno-stack-protector -fpic -fshort-wchar -mno-red-zone -Wall")

include_directories(include)

set(HEADERS
        include/x86/cpuid.h
        include/x86/cr.h
        include/types.h
        include/x86/segment.h
        include/x86/intrinsic.h
        include/macros.h
        include/x86/msr.h)

set(ASM_SOURCES
        src/x86/intrinsic/iregs.asm
        src/x86/intrinsic/iseg.asm
        src/x86/intrinsic/imsr.asm
        src/x86/intrinsic/icpuid.asm)

set(SOURCES
        src/x86/cpuid.cpp
        src/x86/segment.cpp
        src/x86/cr.cpp
        ${ASM_SOURCES} src/x86/msr.cpp include/x86/memory.h)

add_library(arch_static STATIC ${HEADERS} ${SOURCES})
target_compile_definitions(arch_static PUBLIC _DEBUG)
