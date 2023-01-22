# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/ed/code/pico/pico-sdk")

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(MuKOB C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

add_subdirectory(lib/oled1306_i2c)

include_directories(./lib/oled1306_i2c)

# Add executable. Default name is the project name, version 0.1
add_executable(MuKOB MuKOB.c )
pico_set_program_name(MuKOB "MuKOB")
pico_set_program_version(MuKOB "0.1")

pico_enable_stdio_uart(MuKOB 1)
pico_enable_stdio_usb(MuKOB 0)

# Add the standard library to the build
target_link_libraries(MuKOB
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(MuKOB PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(MuKOB
        oled1306_i2c
        hardware_spi
        hardware_i2c
        hardware_pio
        hardware_timer
        hardware_clocks
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
        )

pico_add_extra_outputs(MuKOB)

