# Library: display
add_library(display 
  display.c display.h
  font.c font.h)

# Add the standard library to the build
target_link_libraries(display
        oled1306_i2c
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(display PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)
