# Library: mukob test (obj only)
add_library(mukob_test INTERFACE)

target_sources(mukob_test INTERFACE
    test.c
)

target_link_libraries(mukob_test INTERFACE
    pico_stdlib
)
