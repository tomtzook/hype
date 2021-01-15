
include_directories(../arch/include)

add_executable(hypervisor main.cpp)
target_link_libraries(hypervisor arch_static)
