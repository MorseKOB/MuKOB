# Library: display (obj only)
add_library(ili9341_spi INTERFACE)

target_sources(ili9341_spi INTERFACE
  display_ili9341.c
  ili9341_spi.c
)

target_link_libraries(ili9341_spi INTERFACE
  pico_stdlib
)
