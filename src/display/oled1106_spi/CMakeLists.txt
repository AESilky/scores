add_library(display INTERFACE)

target_sources(display INTERFACE
  display_oled1106.c
  oled1106_spi.c
)

target_link_libraries(display INTERFACE
  display_spi_ops
  pico_stdlib
  hardware_spi
)
