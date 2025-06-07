
set(LINK_FLAGS -target x86_64-pc-windows-msvc -nostdlib -Wl,-entry:EfiMain -Wl,-subsystem:efi_application -fuse-ld=lld-link)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(hype
        main.cpp
        hype.cpp
        hype.h
        debug.h
        debug.cpp
        crt.h
        crt.cpp
        environment.h
        base.h
        status.h
        status.cpp
        memory.h
        memory.cpp
        efi_base.h
        environment.cpp
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
        framework/buffer.h
        framework/buffer.cpp
        framework/result.h
        framework/result.cpp
        framework/ptr.h
        framework/ptr.cpp)

target_compile_definitions(hype PUBLIC _DEBUG)
target_link_options(hype PRIVATE ${LINK_FLAGS})
target_link_libraries(hype PRIVATE edk2 arch)
target_include_directories(hype PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(hype PROPERTIES SUFFIX ".efi")
