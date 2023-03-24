# Library: kob (obj only)
add_library(kob INTERFACE)

target_sources(kob INTERFACE
  kob.c
)

target_link_libraries(kob INTERFACE
  pico_stdlib
)
