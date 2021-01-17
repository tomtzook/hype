set(CMAKE_CXX_FLAGS "-fno-stack-protector -fpic -fshort-wchar -mno-red-zone -Wall")

include_directories(../common/include)
include_directories(include)

set(HEADERS
        include/x86/cpuid.h
        include/x86/cr.h
        include/x86/segment.h
        include/x86/intrinsic.h
        include/x86/msr.h
        include/x86/vmx/environment.h)

set(ASM_SOURCES
        src/x86/intrinsic/iregs.asm
        src/x86/intrinsic/iseg.asm
        src/x86/intrinsic/imsr.asm
        src/x86/intrinsic/icpuid.asm)

set(SOURCES
        src/x86/cpuid.cpp
        src/x86/segment.cpp
        src/x86/cr.cpp
        ${ASM_SOURCES} src/x86/msr.cpp include/x86/memory.h src/x86/vmx/environment.cpp)

add_library(arch_obj OBJECT ${HEADERS} ${SOURCES})
target_compile_definitions(arch_obj PUBLIC _DEBUG)
