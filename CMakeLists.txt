cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(air_quality_monitor VERSION 0.1.0)

target_sources(app PRIVATE
    src/main.c
    src/sgp30.c
    src/ui/ui.c
    src/ui/images.c
    src/ui/screens.c
    src/ui/styles.c
)

# EEZ Studio generates files into src/ui/ — make it includable
target_include_directories(app PRIVATE src/ui)