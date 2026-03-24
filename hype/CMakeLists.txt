include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(hype
        main.cpp
        hype.cpp
        hype.h
        context.h
        interrupts.h
        interrupts.cpp
        interrupts.asm
        cpu.h
        cpu.cpp
        cpu.asm
        vmx/vmx.h
        vmx/vmx.cpp
        vmx/vmexit.asm
        vmx/vmexit.cpp
        vmx/vmexit.h
        vmx/vmentry.h
        vmx/vmentry.asm
        vmx/vmentry_checks.cpp
        debug/hype_gdbstub.cpp
        debug/hype_gdbstub.h
        debug/modules.h
        debug/print.h
        debug/modules.cpp
        debug/print.cpp
        debug/print.h
        memory/mapping.h
        memory/stack.h
        memory/paging.h
        memory/segments.h
        memory/ept.h
        memory/segments.cpp
        memory/paging.cpp
        memory/ept.cpp
        memory/stack.cpp)

target_link_libraries(hype PRIVATE edk2 arch ministl environment gdbstub pe)
target_include_directories(hype PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(hype PROPERTIES SUFFIX ".efi")
