    cmake_minimum_required(VERSION 3.13)

    set(CMAKE_BUILD_TYPE Debug)
    set(PICO_BOARD adafruit_qtpy_rp2040)

    include(pico_sdk_import.cmake)

    project(dsul C CXX ASM)

    pico_sdk_init()

if (TARGET tinyusb_device)
    add_subdirectory(../pico_neopixels build)
    add_executable(dsul
        dsul.cpp
    )

    pico_enable_stdio_usb(dsul 1)
    pico_enable_stdio_uart(dsul 0)

    target_link_libraries(dsul pico_stdlib pico_stdio pico_neopixel)

    # create map/bin/hex file etc.
    pico_add_extra_outputs(dsul)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building dsul because TinyUSB submodule is not initialized in the SDK")
endif()