include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(hype
        main.cpp
        hype.cpp
        hype.h
        base.h
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
        efi/efi_environment.cpp
        efi/efi_debug.cpp
        efi/efi_heap.cpp)

target_link_libraries(hype PRIVATE edk2 arch framework)
target_include_directories(hype PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(hype PROPERTIES SUFFIX ".efi")
