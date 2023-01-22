# Library: oled1306_i2i
if (NOT TARGET oled1306_i2c)
    add_library(oled1306_i2c INTERFACE)

    target_sources(oled1306_i2c INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/oled1306_i2c.c
            ${CMAKE_CURRENT_LIST_DIR}/font.c)
    target_link_libraries(oled1306_i2c INTERFACE pico_stdlib)
endif()
