cmake_minimum_required(VERSION 3.27)

project(VReactor)

set(CMAKE_CXX_STANDARD 17)


include_directories(
        ${CMAKE_CURRENT_LIST_DIR}/src/memory
        ${CMAKE_CURRENT_LIST_DIR}/src/timer)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/memory)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/timer)
