set(CMAKE_CXX_FLAGS "-fno-stack-protector -fpic -fshort-wchar -mno-red-zone -Wall")

include_directories(include)

set(HEADERS
        include/macros.h
        include/types.h
        include/crt.h
        include/debug.h
        include/environment.h
        include/error.h)

set(SOURCES
        src/crt.cpp src/error.cpp src/debug.cpp)

add_library(common_obj OBJECT ${HEADERS} ${SOURCES})
target_compile_definitions(common_obj PUBLIC _DEBUG)
