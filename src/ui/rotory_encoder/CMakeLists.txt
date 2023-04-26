# Library: rotory_encoder (Includes Programable I/O)
add_library(rotory_encoder INTERFACE)

pico_generate_pio_header(
    rotory_encoder ${CMAKE_CURRENT_LIST_DIR}/quadrature_encoder.pio
)

target_sources(rotory_encoder INTERFACE
    rotory_encoder.c
    re_pbsw.c
)

target_link_libraries(rotory_encoder INTERFACE
    pico_stdlib
    hardware_pio
)
