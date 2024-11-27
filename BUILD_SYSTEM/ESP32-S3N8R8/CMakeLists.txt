# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ESP32-S3N8R8_FlintJVM)

add_custom_command(
    TARGET app
    POST_BUILD
    COMMAND $ENV{IDF_PATH}/components/fatfs/wl_fatfsgen.py
    --output_file storage.bin
    --partition_size 0x6F0000
    --sector_size 512
    --long_name_support
    ../../Resources/ESPJDK
)

add_custom_command(
    TARGET app
    POST_BUILD
    COMMAND ../../Tools/esptool-win64/esptool.exe
    --chip esp32s3 merge_bin --flash_mode dio --flash_size 8MB --flash_freq 80m
    -o ../../build/ESP32-S3N8R8_FlintJVM_Release.bin
    0x0 bootloader/bootloader.bin
    0x8000 partition_table/partition-table.bin
    0x10000 ESP32-S3N8R8_FlintJVM.bin
    0x110000 storage.bin
)

