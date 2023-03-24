# Library: display (obj only)
add_library(display INTERFACE)

target_sources(display INTERFACE
    display.c
    font_10_16.c
)

# Use one of the two displays
add_subdirectory(ili9341_spi)

#  add_subdirectory(oled1306_i2c)
target_link_libraries(display INTERFACE
    pico_stdlib
)
