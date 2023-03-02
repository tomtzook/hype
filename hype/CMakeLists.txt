
list(APPEND COMPILE_FLAGS "-fno-stack-protector" "-fshort-wchar" "-mno-red-zone" "-Wall" "-DEFI_FUNCTION_WRAPPER")
set(LINK_FLAGS -target x86_64-unknown-windows -nostdlib -Wl,-entry:EfiMain -Wl,-subsystem:efi_application -fuse-ld=lld-link)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(hype
        main.cpp
        hype.cpp hype.h
        common/result.h
        common/hype_defs.h
        environment/allocation.h
        environment/allocation.cpp
        common/result.cpp
        common/common.h
        efi/memory.h
        efi/memory.cpp
        efi/commonefi.h
        efi/commonefi.cpp
        common/debug.h
        common/debug.cpp environment/paging.h environment/paging.cpp environment/memory.h environment/memory.cpp common/crt.h common/crt.cpp common/hype_types.h)

target_compile_definitions(hype PUBLIC _DEBUG)
target_compile_options(hype PRIVATE ${COMPILE_FLAGS})
target_link_options(hype PRIVATE ${LINK_FLAGS})
target_link_libraries(hype PRIVATE edk2 arch)
set_target_properties(hype PROPERTIES SUFFIX ".efi")
