cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(vm)

set(CMAKE_CXX_STANDARD 17)

file(GLOB VM_SRC
    "src/*.h"
    "src/*.cpp"
)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)
add_executable(assembler ${VM_SRC})