cmake_minimum_required(VERSION 3.17)
project(hypervisor)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)
set(HEADERS include/x64/id.h include/x64/regs.h include/types.h include/x64/gdt.h)
set(SOURCES src/x64/id.cpp src/x64/gdt.cpp include/x64/intrinsic.h src/x64/regs.cpp)

add_executable(hypervisor main.cpp ${HEADERS} ${SOURCES})
