
find_package(GNUefi REQUIRED)

include_directories(../arch/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -DGNU_EFI_USE_MS_ABI -m64 -ffreestanding -Wall -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -Wall")
set(LDFLAGS "-nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -Wl,-eefi_main -lgcc -Wl,-Bsymbolic -Wl,-nostdlib")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LDFLAGS}")

set(HEADERS
        include/debug.h)

set(SOURCES
        src/main.cpp)

set(TARGET_NAME hypervisor)

add_library(${TARGET_NAME} SHARED ${HEADERS} ${SOURCES})
target_compile_definitions(${TARGET_NAME} PUBLIC _DEBUG)
target_link_libraries(${TARGET_NAME} PUBLIC GnuEfi::GnuEfi PRIVATE ${GNUEFI_LIBRARIES} ${LDFLAGS})

add_custom_command(TARGET "${TARGET_NAME}" POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib${TARGET_NAME}.dll"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}.efi")
