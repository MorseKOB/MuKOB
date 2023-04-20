# Library: mks (obj only)
add_library(mks INTERFACE)

target_sources(mks INTERFACE
  mks.c
)

target_link_libraries(mks INTERFACE
  pico_stdlib
)
