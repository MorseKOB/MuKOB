# Library: display (obj only)
add_library(display INTERFACE)

target_sources(display INTERFACE
  display_ili9341.c
  font_10_16.c
  ili9341_spi.c
)

target_link_libraries(display INTERFACE
  pico_stdlib
)
