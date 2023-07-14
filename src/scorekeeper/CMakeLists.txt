# Library: Back-End (BE) (obj only)
add_library(scorekeeper INTERFACE)

target_sources(scorekeeper INTERFACE
    scorekeeper.c
)

target_link_libraries(be INTERFACE
    pico_stdlib
)
