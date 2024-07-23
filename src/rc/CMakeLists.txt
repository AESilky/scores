# Library: term (terminal) (obj only)
add_library(rc INTERFACE)

target_sources(rc INTERFACE
    rc.c
)

target_link_libraries(rc INTERFACE
    pico_stdlib
)
