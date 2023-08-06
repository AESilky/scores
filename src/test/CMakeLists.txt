# Library: scores test (obj only)
add_library(scores_test INTERFACE)

target_sources(scores_test INTERFACE
    test.c
)

target_link_libraries(scores_test INTERFACE
    pico_stdlib
)
