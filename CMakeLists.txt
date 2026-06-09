cmake_minimum_required(VERSION 3.20)
project(vm_scheduling_energy_simgrid CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(SimGrid REQUIRED)

add_executable(simulation src/main.cpp)
target_link_libraries(simulation PUBLIC ${SimGrid_LIBRARY})
target_include_directories(simulation PUBLIC ${SimGrid_INCLUDE_DIR})
