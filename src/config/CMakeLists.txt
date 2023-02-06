# Library: config (obj only)
add_library(config INTERFACE)

target_sources(config INTERFACE
  config.c
)

target_link_libraries(config INTERFACE
  pico_stdlib
)
