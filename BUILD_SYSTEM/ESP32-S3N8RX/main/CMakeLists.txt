set(priv_requires fatfs console esp_timer nvs_flash esp_wifi)

if(CONFIG_EXAMPLE_STORAGE_MEDIA_SPIFLASH)
    list(APPEND priv_requires wear_levelling esp_partition)
endif()

idf_component_register(
    SRCS "../../../ESPJVM/User/Src/main.cpp"
    SRCS "../../../ESPJVM/User/Src/esp_board_esp32_s3.cpp"
    SRCS "../../../ESPJVM/User/Src/esp_usb_device.c"
    SRCS "../../../ESPJVM/User/Src/esp_usb_cdc_debugger.cpp"

    SRCS "../../../ESPJVM/SystemAPI/Src/esp_system_api.cpp"
    SRCS "../../../ESPJVM/SystemAPI/Src/esp_system_io_api.cpp"
    SRCS "../../../ESPJVM/SystemAPI/Src/esp_system_thread_api.cpp"

    SRCS "../../../ESPJVM/Native/Src/esp_system_native.cpp"
    SRCS "../../../ESPJVM/Native/Src/esp_system_native_pin.cpp"
    SRCS "../../../ESPJVM/Native/Src/esp_system_native_spi.cpp"
    SRCS "../../../ESPJVM/Native/Src/esp_system_native_port.cpp"
    SRCS "../../../ESPJVM/Native/Src/esp_system_native_wifi.cpp"

    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_class_loader.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_common.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_const_name_base.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_const_name_base_hash_table.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_const_pool.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_debugger.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_execution.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_field_info.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_fields_data.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_method_info.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_array_object.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_const_utf8_binary_tree.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_string_binary_tree.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_class_binary_tree.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_class_data_binary_tree.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_java_class.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_java_object.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_java_thread.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_java_string.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_java_throwable.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_java_boolean.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_java_byte.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_java_character.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_java_short.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_java_integer.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_java_float.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_java_long.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_java_double.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_mutex.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_throw_support.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_character_class.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_class_class.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_class.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_double_class.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_float_class.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_graphics_class.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_math_class.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_object_class.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_thread_class.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_print_stream_class.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_string_class.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_system_class.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_biginteger_class.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_array_class.cpp"

    INCLUDE_DIRS "../../../ESPJVM/User/Inc"
    INCLUDE_DIRS "../../../ESPJVM/Native/Inc"
    INCLUDE_DIRS "../../../ESPJVM/SystemAPI/Inc"
    INCLUDE_DIRS "../../../ESPJVM/FlintJVM/VM/Inc"
    INCLUDE_DIRS "../../../ESPJVM/FlintJVM/Native/Inc"

    PRIV_REQUIRES "${priv_requires}"
)

add_compile_definitions(MULTI_HEAP_FREERTOS)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-address)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-volatile)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-uninitialized)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-strict-aliasing)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-implicit-fallthrough)
