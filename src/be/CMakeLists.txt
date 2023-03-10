# Library: Back-End (BE) (obj only)
add_library(be INTERFACE)

target_sources(be INTERFACE
    be.c
)

#add_subdirectory(display)

target_link_libraries(be INTERFACE
    pico_stdlib
)
