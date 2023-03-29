# Library: data (obj only)
add_library(data INTERFACE)

target_sources(data INTERFACE
  morse_tables.c
)
