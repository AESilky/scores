add_library(display_spi_ops INTERFACE)

target_sources(display_spi_ops INTERFACE
    display_spi_ops.c
)

add_subdirectory(fonts)
add_subdirectory(oled1106_spi)

target_link_libraries(display_spi_ops INTERFACE
    pico_stdlib
    hardware_spi
)

