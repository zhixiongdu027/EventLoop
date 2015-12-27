


set(EXAMPLE_SINGLETON_SOURCE main.cpp)
add_executable(example_singleton ${EXAMPLE_SINGLETON_SOURCE})
target_link_libraries(example_singleton eventloop)