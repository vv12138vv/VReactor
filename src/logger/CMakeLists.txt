set(LOGGER_SRC logger.cpp log_stream.cpp async_log.cpp ./log_file/file_util.cpp ./log_file/log_file.cpp)

add_library(logger_lib STATIC ${LOGGER_SRC})

add_executable(test_logger ./test/test_logger.cpp ../base/thread.cpp)
target_link_libraries(test_logger logger_lib)