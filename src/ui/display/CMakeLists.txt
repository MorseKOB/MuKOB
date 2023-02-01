# Library: display (obj only)
add_library(display INTERFACE)

target_sources(display INTERFACE
  display.c
  font_9_10_h.c
  ili9341_spi.c
)

target_link_libraries(display INTERFACE
  pico_stdlib
)
