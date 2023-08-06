add_library(segments7 INTERFACE)

target_sources(segments7 INTERFACE
  segments7.c
  font_7segment.c
)

target_link_libraries(segments7 INTERFACE
  pico_stdlib
)
