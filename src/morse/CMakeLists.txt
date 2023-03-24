# Library: morse (obj only)
add_library(morse INTERFACE)

target_sources(morse INTERFACE
  morse.c
)

target_link_libraries(morse INTERFACE
  pico_stdlib
)
