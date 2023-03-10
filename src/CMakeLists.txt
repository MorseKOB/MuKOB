# MuKOB primary project file

cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/ed/code/pico/pico-sdk")
set(PICO_BOARD pico_w CACHE STRING "Board type")
set(PICO_USE_MALLOC_MUTEX 1)
# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()
# Use fully-deoptimized debug build for true single-step and data viewing. Set to '0' for some optimizations
set(PICO_DEOPTIMIZED_DEBUG 1)
pico_sdk_init()

project(MuKOB C CXX ASM)

add_compile_options(
  -Wall
  -Wno-format                     # int != int32_t as far as the compiler is concerned
  -Wno-unused-function
  -Wno-maybe-uninitialized
  #-H                              # list include header paths
)

# External libraries
add_subdirectory(lib/sd_card)
# Local libraries (additional sources)
add_subdirectory(be)
add_subdirectory(config)
add_subdirectory(cmt)
add_subdirectory(kob)
add_subdirectory(net)
add_subdirectory(test)
add_subdirectory(util)
add_subdirectory(ui)

# Add executable.
add_executable(MuKOB
  MuKOB.c
  mkboard.c
  spi_ops.c
)

pico_set_program_name(MuKOB "MuKOB")
pico_set_program_version(MuKOB "0.1")

# Use the UART while using the Picoprobe
# For Release the USB will be better
pico_enable_stdio_uart(MuKOB 1)
pico_enable_stdio_usb(MuKOB 0)

# Add the required include file paths to the build
include_directories(
  ${CMAKE_CURRENT_LIST_DIR}
  ${PICO_SDK_PATH}/lib/lwip/src/include
  ${PICO_SDK_PATH}/src/common/pico_stdlib/include
  ${PICO_SDK_PATH}/src/rp2_common/hardware_dma/include
  ${PICO_SDK_PATH}/src/rp2_common/hardware_spi/include
  ${PICO_SDK_PATH}/src/rp2_common/hardware_i2c/include
  ${PICO_SDK_PATH}/src/rp2_common/hardware_pio/include
  ${PICO_SDK_PATH}/src/rp2_common/hardware_rtc/include
  ${PICO_SDK_PATH}/src/rp2_common/pico_cyw43_arch/include
  ${PICO_SDK_PATH}/src/rp2_common/pico_multicore/include
  be
  config
  cmt
  kob
  net
  test
  ui
  ui/display
  ui/display/ili9341_spi
  ui/rotory_encoder
  ui/term
  util
  lib/sd_card/sd_driver
  lib/sd_card/ff15/source
)

# ZZZ Figure out how to set `PICO_DEBUG_MALLOC` only if 'debug build'
add_compile_definitions(
  PICO_USE_MALLOC_MUTEX
#  PICO_DEBUG_MALLOC
)

# Add the libraries required by the system to the build
target_link_libraries(MuKOB
  be
  config
  cmt
  display
  ili9341_spi
  kob
  net
  mukob_test
  term
  ui
  util
  hardware_spi
  hardware_i2c
  hardware_pio
  hardware_timer
  hardware_clocks
  pico_cyw43_arch_lwip_threadsafe_background
  pico_multicore
  pico_stdlib
  SD_FatFs
)

pico_add_extra_outputs(MuKOB)
