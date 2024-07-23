# Library: Score Keeper (obj only)
add_library(scorekeeper INTERFACE)

target_sources(scorekeeper INTERFACE
    scorekeeper.c
    sk_screen.c
    sk_tod.c
)

target_link_libraries(scorekeeper INTERFACE
    display
    pico_stdlib
)
