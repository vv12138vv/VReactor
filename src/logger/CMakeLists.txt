set(LOGGER_SRC logger.cpp log_stream.cpp)

add_executable(test_logger ./test/test_logger.cpp ${LOGGER_SRC})
