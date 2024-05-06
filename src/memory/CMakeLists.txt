set(MEMORY_SRC memory_pool.cpp)

add_library(memory_pool_lib STATIC ${MEMORY_SRC})

add_subdirectory(./test)