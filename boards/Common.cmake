
set(COMMON_SRCS
    "../../../espjvm/core/src/main.cpp"
    "../../../espjvm/system_api/src/esp_system_api.cpp"

    "../../../espjvm/native/src/esp_native.cpp"
    "../../../espjvm/native/src/esp_native_common.cpp"
    "../../../espjvm/native/src/esp_native_adc.cpp"
    "../../../espjvm/native/src/esp_native_dac.cpp"
    "../../../espjvm/native/src/esp_native_pin.cpp"
    "../../../espjvm/native/src/esp_native_touch_pad.cpp"
    "../../../espjvm/native/src/esp_native_onewire.cpp"
    "../../../espjvm/native/src/esp_native_bitstream.cpp"
    "../../../espjvm/native/src/esp_native_spi_master.cpp"
    "../../../espjvm/native/src/esp_native_i2c_master.cpp"
    "../../../espjvm/native/src/esp_native_i2s_master.cpp"
    "../../../espjvm/native/src/esp_native_uart.cpp"
    "../../../espjvm/native/src/esp_native_port.cpp"
    "../../../espjvm/native/src/esp_native_wifi.cpp"
)

set (COMMON_INCS
    "../../../espjvm/core/inc"
    "../../../espjvm/native/inc"
    "../../../espjvm/system_api/inc"
)
