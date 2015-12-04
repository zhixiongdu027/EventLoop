


set(EXAMPLE_TASK_SOURCE main.cpp)
add_executable(example_task ${EXAMPLE_TASK_SOURCE})
target_link_libraries(example_task eventloop)