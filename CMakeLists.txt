cmake_minimum_required(VERSION 3.20.0)

# Explicitly declare the board overlay so it is picked up regardless of the
# board-qualifier naming convention (esp32c6_devkitc/esp32c6/hpcore).
# Zephyr also auto-discovers boards/esp32c6_devkitc_esp32c6_hpcore.overlay,
# but being explicit here avoids any ambiguity.
set(DTC_OVERLAY_FILE
  "${CMAKE_CURRENT_SOURCE_DIR}/boards/esp32c6_devkitc_esp32c6_hpcore.overlay"
)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(air_quality_monitor VERSION 1.0.0)

target_sources(app PRIVATE
  src/main.c
  src/ui/ui.c
  src/sensors/sgp30.c
)

# Include directories
target_include_directories(app PRIVATE
  src
  src/ui
  src/sensors
)