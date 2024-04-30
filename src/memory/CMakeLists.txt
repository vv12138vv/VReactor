set(MEMORY_SRC memory_pool.cpp)

add_library(memory_pool ${MEMORY_SRC})


add_executable(memory_pool_test ${CMAKE_CURRENT_LIST_DIR}/test/memory_pool_test.cpp)

target_link_libraries(memory_pool_test memory_pool)