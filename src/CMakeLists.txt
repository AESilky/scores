# scores primary project file

cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_BOARD pico_w CACHE STRING "Board type")
set(PICO_USE_MALLOC_MUTEX 1)
# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
if (PICO_SDK_VERSION_STRING VERSION_LESS "1.5.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.5.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()
# Use fully-deoptimized debug build for true single-step and data viewing. Set to '0' for some optimizations
set(PICO_DEOPTIMIZED_DEBUG 1)

project(scores C CXX ASM)

add_compile_options(
  -O0                       # No optimizations for now, for debugging
  -Wall
  -Wno-format               # int != int32_t as far as the compiler is concerned
  -Wno-unused-function
  -Wno-maybe-uninitialized
  #-H                       # list include header paths
)
pico_sdk_init()

# External libraries
add_subdirectory(lib/sd_card)
# Local libraries (additional sources)
add_subdirectory(be)
add_subdirectory(config)
add_subdirectory(cmt)
add_subdirectory(gfx)
add_subdirectory(net)
add_subdirectory(scorekeeper)
add_subdirectory(test)
add_subdirectory(util)
add_subdirectory(ui)

# Add executable.
add_executable(scores
  main.c
  board.c
  debug_support.c
  spi_ops.c
)

pico_set_program_name(scores "Scores")
pico_set_program_version(scores "0.1")

# Use the UART while using the Picoprobe
# For Release the USB will be better
pico_enable_stdio_uart(scores 1)
pico_enable_stdio_usb(scores 0)

# Add the required include file paths to the build
include_directories(
  ${CMAKE_CURRENT_LIST_DIR}
  ${PICO_SDK_PATH}/lib/lwip/src/include
  ${PICO_SDK_PATH}/src/common/pico_stdlib/include
  ${PICO_SDK_PATH}/src/rp2_common/hardware_adc/include
  ${PICO_SDK_PATH}/src/rp2_common/hardware_dma/include
  ${PICO_SDK_PATH}/src/rp2_common/hardware_spi/include
  ${PICO_SDK_PATH}/src/rp2_common/hardware_i2c/include
  ${PICO_SDK_PATH}/src/rp2_common/hardware_pio/include
  ${PICO_SDK_PATH}/src/rp2_common/hardware_pwm/include
  ${PICO_SDK_PATH}/src/rp2_common/hardware_rtc/include
  ${PICO_SDK_PATH}/src/rp2_common/pico_cyw43_arch/include
  ${PICO_SDK_PATH}/src/rp2_common/pico_multicore/include
  ${PICO_SDK_PATH}/src/rp2040/hardware_structs/include
  lib/sd_card/sd_driver
  lib/sd_card/ff15/source
)

# ZZZ Figure out how to set `PICO_DEBUG_MALLOC` only if 'debug build'
add_compile_definitions(
  PICO_MALLOC_PANIC
  PICO_USE_MALLOC_MUTEX
  PICO_USE_STACK_GUARDS
  PICO_STACK_SIZE=4096
  PICO_CORE1_STACK_SIZE=4096
  # PICO_DEBUG_MALLOC
)

# Add the libraries required by the system to the build
target_link_libraries(scores
  be
  config
  cmd
  cmt
  display
  gfx
  scores_test
  net
  score_panel
  scorekeeper
  segments7
  term
  ui
  util
  hardware_adc
  hardware_clocks
  hardware_exception
  hardware_i2c
  hardware_pio
  hardware_spi
  hardware_timer
  pico_cyw43_arch_lwip_threadsafe_background
  pico_multicore
  pico_stdlib
  SD_FatFs
)

pico_set_linker_script(scores ${CMAKE_SOURCE_DIR}/memmap_scores_custom.ld)

pico_add_extra_outputs(scores)
