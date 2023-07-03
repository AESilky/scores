# Library: net (obj only)
add_library(net INTERFACE)

target_sources(net INTERFACE
  net.c
)

target_link_libraries(net INTERFACE
  pico_stdlib
)
