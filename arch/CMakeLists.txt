set(CMAKE_CXX_FLAGS "-fno-stack-protector -fpic -fshort-wchar -mno-red-zone -Wall")

include_directories(../common/include)
include_directories(include)

set(HEADERS
        include/x86/cpuid.h
        include/x86/cr.h
        include/x86/segment.h
        include/x86/intrinsic.h
        include/x86/msr.h
        include/x86/vmx/environment.h
        include/x86/error.h
        include/x86/memory.h include/x86/vmx/operations.h include/x86/vmx/instrinsic.h include/x86/paging.h)

set(ASM_SOURCES
        src/x86/intrinsic/igen.asm
        src/x86/intrinsic/iregs.asm
        src/x86/intrinsic/iseg.asm
        src/x86/intrinsic/imsr.asm
        src/x86/intrinsic/icpuid.asm)

set(SOURCES
        src/x86/cpuid.cpp
        src/x86/segment.cpp
        src/x86/cr.cpp
        ${ASM_SOURCES}
        src/x86/msr.cpp
        src/x86/vmx/environment.cpp
        src/x86/error.cpp
        src/x86/memory.cpp src/x86/vmx/operations.cpp src/x86/vmx/intrinsic.cpp src/x86/paging.cpp)

add_library(arch_obj OBJECT ${HEADERS} ${SOURCES})
target_compile_definitions(arch_obj PUBLIC _DEBUG)
