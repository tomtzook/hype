
add_library(framework STATIC
    empty.cpp
    optional.h
        result.h
    common.h
    type_traits.h
        environment.h
        status.h
        types.h
        buffer.h
        crt.h
        crt.cpp
        heap.h
        check.h
        ptr.h
)
target_link_libraries(framework PRIVATE arch)
target_include_directories(framework PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
