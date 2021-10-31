
list(APPEND COMPILE_FLAGS "-fno-stack-protector" "-fshort-wchar" "-mno-red-zone" "-Wall" "-DEFI_FUNCTION_WRAPPER")
# set(LINK_FLAGS -nostdlib -T ${EFI_LDS} -shared -Bsymbolic -L ${EFI_LIB_PATH} -L ${LIB_PATH} ${EFI_CRT_OBJS})
set(LINK_FLAGS -target x86_64-unknown-windows -nostdlib -Wl,-entry:efi_main -Wl,-subsystem:efi_application -fude-ld=lld-link)

#include_directories(${EFI_INCLUDE_PATH} ${EFI_INCLUDE_PATH}/${ARCH} ${EFI_INCLUDE_PATH}/protocol)
include_directories(../common/include)
include_directories(../arch/include)
include_directories(include)

set(HEADERS
        include/commonefi.h
        include/hype.h include/common.h include/vcpu.h include/env.h include/vmcs.h)

set(SOURCES
        src/common/environment.cpp
        src/common/debug.cpp
        src/main.cpp
        src/efi.cpp
        src/hype.cpp src/vcpu.cpp src/env.cpp src/vmcs.cpp)

set(TARGET_NAME hype)

add_library(${TARGET_NAME} SHARED
        ${HEADERS}
        ${SOURCES}
        $<TARGET_OBJECTS:common_obj>
        $<TARGET_OBJECTS:arch_obj>
        )

target_compile_definitions("${TARGET_NAME}" PUBLIC _DEBUG)
target_link_options(${TARGET_NAME} PRIVATE ${LINK_FLAGS})
target_link_libraries(${TARGET_NAME} PRIVATE efi gnuefi)
