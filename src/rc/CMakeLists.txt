# Library: rc (remote control) (obj only)
add_library(rc INTERFACE)

pico_generate_pio_header(rc ${CMAKE_CURRENT_LIST_DIR}/nec-rx.pio)

target_sources(rc INTERFACE
    rc.c
)

target_link_libraries(rc INTERFACE
    pico_stdlib
    hardware_pio
)
