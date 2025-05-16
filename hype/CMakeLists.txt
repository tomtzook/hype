
list(APPEND CXX_COMPILE_FLAGS "-fno-stack-protector" "-fshort-wchar" "-mno-red-zone" "-Wall" "-DEFI_FUNCTION_WRAPPER")
set(LINK_FLAGS -target x86_64-unknown-windows -nostdlib -Wl,-entry:EfiMain -Wl,-subsystem:efi_application -fuse-ld=lld-link)

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
        cpu.h
        cpu.cpp
        cpu.asm
        vmexit.asm
        vmexit.cpp
        vmexit.h
        vmentry.h
        vmentry.asm
        vmentry_checks.cpp)

target_compile_definitions(hype PUBLIC _DEBUG)
target_compile_options(hype PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CXX_COMPILE_FLAGS}>)
target_link_options(hype PRIVATE ${LINK_FLAGS})
target_link_libraries(hype PRIVATE edk2 arch)
set_target_properties(hype PROPERTIES SUFFIX ".efi")
