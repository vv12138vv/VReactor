cmake_minimum_required(VERSION 3.20)

project(VReactor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/src/memory
  ${CMAKE_CURRENT_LIST_DIR}/src/timer
  ${CMAKE_CURRENT_LIST_DIR}/src/net
  ${CMAKE_CURRENT_LIST_DIR}/src/net/poller
  ${CMAKE_CURRENT_LIST_DIR}/src/net/base
  ${CMAKE_CURRENT_LIST_DIR}/src/logger
  ${CMAKE_CURRENT_LIST_DIR}/src/logger/log_file
  ${CMAKE_CURRENT_LIST_DIR}/src/base)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/memory)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/timer)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/net)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/logger)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/base)
