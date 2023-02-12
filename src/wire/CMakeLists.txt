# Library: net (obj only)
add_library(wire INTERFACE)

target_sources(wire INTERFACE
  net.c
  mkwire.c
)

include_directories(
  ${PICO_SDK_PATH}/src/rp2_common/hardware_rtc/include
)

target_link_libraries(wire INTERFACE
  pico_stdlib
)
