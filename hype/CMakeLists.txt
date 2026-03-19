include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(hype
        main.cpp
        hype.cpp
        hype.h
        memory.h
        memory.cpp
        context.h
        vmx.h
        vmx.cpp
        interrupts.h
        interrupts.cpp
        interrupts.asm
        cpu.h
        cpu.cpp
        cpu.asm
        vmexit.asm
        vmexit.cpp
        vmexit.h
        vmentry.h
        vmentry.asm
        vmentry_checks.cpp
        hype_gdbstub.cpp
        hype_gdbstub.h
        debug/modules.h
        debug/print.h
        debug/modules.cpp
        debug/print.cpp
        debug/print.h)

target_link_libraries(hype PRIVATE edk2 arch ministl environment gdbstub pe)
target_include_directories(hype PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(hype PROPERTIES SUFFIX ".efi")
