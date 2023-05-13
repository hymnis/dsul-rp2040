cmake_minimum_required(VERSION 3.13)

#set(CMAKE_BUILD_TYPE Debug) # create a debug build
set(PICO_BOARD adafruit_qtpy_rp2040) # use Adafruit QT PY RP2040 instead of RPi Pico

include(pico_sdk_import.cmake)

project(dsul C CXX ASM)

pico_sdk_init()


add_subdirectory(../pico_neopixels build)
add_executable(dsul
    dsul.cpp
)

pico_enable_stdio_usb(dsul 1) # enable serial over USB
pico_enable_stdio_uart(dsul 0) # disable serial over UART

target_link_libraries(dsul pico_stdlib pico_stdio hardware_pio pico_neopixel)

# Create map/bin/hex file etc.
pico_add_extra_outputs(dsul)
