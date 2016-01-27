
set(EXAMPLE_MSGPACK_SOURCE_FILES main.cpp)

add_executable(example_msgpack ${EXAMPLE_MSGPACK_SOURCE_FILES})
target_link_libraries(example_msgpack eventloop msgpackc pthread)
