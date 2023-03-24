add_library(display INTERFACE)

target_sources(display INTERFACE
  display.c
  font_9_10_h.c
  oled1306_i2c.c
)

target_link_libraries(display INTERFACE
  pico_stdlib
  hardware_i2c
)
