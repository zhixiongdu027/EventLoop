


set(EXAMPLE_ECHO_SOURCE main.cpp)
add_executable(example_echo ${EXAMPLE_ECHO_SOURCE})
target_link_libraries(example_echo eventloop)
