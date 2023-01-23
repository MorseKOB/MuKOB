# Library: oled1306_i2i
add_library(oled1306_i2c oled1306_i2c.c)

# pull in common dependencies and additional i2c hardware support
target_link_libraries(oled1306_i2c pico_stdlib hardware_i2c)

# Add the standard include files to the build
target_include_directories(oled1306_i2c PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)
