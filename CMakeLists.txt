cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(vm)

set(CMAKE_CXX_STANDARD 17)

file(GLOB VM_SRC
    "src/*.h"
    "src/*.cpp"
)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)
add_executable(vm ${VM_SRC})

# Link core to main.
#target_link_libraries(main
#  core)

# Add flags.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")