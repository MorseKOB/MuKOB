# Library: UI (obj only)
add_library(ui INTERFACE)

target_sources(ui INTERFACE
    ui.c
    ui_disp.c
    ui_term.c
)

add_subdirectory(cmd)
add_subdirectory(display)
add_subdirectory(rotary_encoder)
add_subdirectory(term)

target_link_libraries(ui INTERFACE
    pico_stdlib
)
