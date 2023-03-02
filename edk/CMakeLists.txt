
if(NOT DEFINED EDK2_PATH)
    message(FATAL_ERROR "Missing EDK2_PATH definition")
endif()

list(APPEND COMPILE_FLAGS "-fno-stack-protector" "-fshort-wchar" "-mno-red-zone" "-Wall" "-DEFI_FUNCTION_WRAPPER")

set(EDK2_HEADERS
        ${EDK2_PATH}/MdePkg/Include
        ${EDK2_PATH}/MdePkg/Include/X64)
file(GLOB EDK2_LIBS
        "${CMAKE_CURRENT_SOURCE_DIR}/../.edk2build/libs/*.lib"
        )

set(EDK_AUTO_GEN ${CMAKE_CURRENT_SOURCE_DIR}/../.edk2build/AutoGen.c)

add_custom_command(
        COMMENT
            "Build EDK2 dependencies"
        OUTPUT
            ${EDK_AUTO_GEN}
        COMMAND
            ${CMAKE_CURRENT_SOURCE_DIR}/../build_edk.sh ${EDK2_PATH}
)

add_library(edk2 STATIC empty.c ${EDK_AUTO_GEN})
target_include_directories(edk2 PUBLIC ${EDK2_HEADERS})
target_compile_options(edk2 PRIVATE ${COMPILE_FLAGS})
target_link_libraries(edk2 ${EDK2_LIBS})
