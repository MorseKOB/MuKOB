# Library: net (obj only)
add_library(net OBJECT
    net.c
)

# Add the required libraries to the build
target_link_libraries(net
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
        )

# Make the include files available
target_include_directories(net PUBLIC .
    ${CMAKE_CURRENT_LIST_DIR}/..
)
