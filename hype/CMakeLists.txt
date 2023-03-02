
list(APPEND COMPILE_FLAGS "-fno-stack-protector" "-fshort-wchar" "-mno-red-zone" "-Wall" "-DEFI_FUNCTION_WRAPPER")
set(LINK_FLAGS -target x86_64-unknown-windows -nostdlib -Wl,-entry:EfiMain -Wl,-subsystem:efi_application -fuse-ld=lld-link)


add_executable(hype
        main.cpp
        )

target_compile_definitions(hype PUBLIC _DEBUG)
target_compile_options(hype PRIVATE ${COMPILE_FLAGS})
target_link_options(hype PRIVATE ${LINK_FLAGS})
target_link_libraries(hype PRIVATE edk2)
set_target_properties(hype PROPERTIES SUFFIX ".efi")
