macro(enable_asm)
    find_program(NASM_BIN nasm)

    if(NOT NASM_BIN)
        set(NASM_BIN "/usr/bin/nasm")
        if(NOT EXISTS ${NASM_BIN})
            message(FATAL_ERROR "Unable to find nasm, or nasm is not installed")
        endif()
    endif()

    execute_process(COMMAND ${NASM_BIN} -v OUTPUT_VARIABLE NASM_ID OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(CMAKE_ASM_NASM_COMPILER_ID ${NASM_ID})

    # set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf64")
    set(CMAKE_ASM_NASM_OBJECT_FORMAT "win64")

    enable_language(ASM_NASM)

    #set(CMAKE_ASM_NASM_FLAGS "-d ${PREFIX} -d ${OSTYPE} -d SYSV")
    #set(CMAKE_ASM_NASM_FLAGS "-d ${PREFIX} -d ${OSTYPE} -d ${ABITYPE}")

    set(CMAKE_ASM_NASM_CREATE_STATIC_LIBRARY TRUE)
endmacro(enable_asm)
