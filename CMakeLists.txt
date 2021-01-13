cmake_minimum_required(VERSION 3.17)
project(hypervisor)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)
set(HEADERS include/cpu/id.h include/cpu/regs.h include/types.h)
set(SOURCES src/cpu/id.cpp)

add_executable(hypervisor main.cpp ${HEADERS} ${SOURCES} main.cpp)
