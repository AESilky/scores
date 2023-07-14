add_library(score_panel INTERFACE)

target_sources(score_panel INTERFACE
  panel.c
)

target_link_libraries(score_panel INTERFACE
  pico_stdlib
)
