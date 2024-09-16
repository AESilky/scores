# Library: setup (obj only)
add_library(setup INTERFACE)

target_sources(setup INTERFACE
    setup.c
)

target_link_libraries(setup INTERFACE
    pico_stdlib
)
