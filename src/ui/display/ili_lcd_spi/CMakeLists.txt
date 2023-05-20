# Library: display (obj only)
add_library(ili_lcd_spi INTERFACE)

target_sources(ili_lcd_spi INTERFACE
  ili_lcd_spi.c
  display_ili.c
)

# Use one of the two displays
add_subdirectory(ili9341_spi)
add_subdirectory(ili9488_spi)

target_link_libraries(ili_lcd_spi INTERFACE
  pico_stdlib
)
