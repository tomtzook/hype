

add_library(environment STATIC
    efi/efi_debug.cpp
    efi/efi_environment.cpp
    efi/efi_heap.cpp
)

target_include_directories(environment PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(environment PRIVATE -nostdinc++ -fno-exceptions)
target_link_options(environment PRIVATE -nostdlib -nolibc -nodefaultlibs)
target_link_libraries(environment ministl edk2 arch)
