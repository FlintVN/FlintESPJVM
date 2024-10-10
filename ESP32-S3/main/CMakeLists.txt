set(priv_requires fatfs console esp_timer nvs_flash)

if(CONFIG_EXAMPLE_STORAGE_MEDIA_SPIFLASH)
    list(APPEND priv_requires wear_levelling esp_partition)
endif()

idf_component_register(
    SRCS "../../ESPJVM/User/Src/main.cpp"
    SRCS "../../ESPJVM/User/Src/esp_board_esp32_s3.cpp"
    SRCS "../../ESPJVM/User/Src/esp_usb_device.c"
    SRCS "../../ESPJVM/User/Src/esp_usb_cdc_debugger.cpp"

    SRCS "../../ESPJVM/SystemAPI/Src/esp_system_api.cpp"
    SRCS "../../ESPJVM/SystemAPI/Src/esp_system_io_api.cpp"
    SRCS "../../ESPJVM/SystemAPI/Src/esp_system_thread_api.cpp"

    SRCS "../../ESPJVM/Native/Src/esp_system_native.cpp"
    SRCS "../../ESPJVM/Native/Src/esp_system_native_pin.cpp"
    SRCS "../../ESPJVM/Native/Src/esp_system_native_port.cpp"

    SRCS "../../ESPJVM/FlintJVM/VM/Src/flint_attribute_info.cpp"
    SRCS "../../ESPJVM/FlintJVM/VM/Src/flint_class_loader.cpp"
    SRCS "../../ESPJVM/FlintJVM/VM/Src/flint_class.cpp"
    SRCS "../../ESPJVM/FlintJVM/VM/Src/flint_common.cpp"
    SRCS "../../ESPJVM/FlintJVM/VM/Src/flint_const_name.cpp"
    SRCS "../../ESPJVM/FlintJVM/VM/Src/flint_const_pool.cpp"
    SRCS "../../ESPJVM/FlintJVM/VM/Src/flint_debugger.cpp"
    SRCS "../../ESPJVM/FlintJVM/VM/Src/flint_execution.cpp"
    SRCS "../../ESPJVM/FlintJVM/VM/Src/flint_field_info.cpp"
    SRCS "../../ESPJVM/FlintJVM/VM/Src/flint_fields_data.cpp"
    SRCS "../../ESPJVM/FlintJVM/VM/Src/flint_load_file_error.cpp"
    SRCS "../../ESPJVM/FlintJVM/VM/Src/flint_find_native_error.cpp"
    SRCS "../../ESPJVM/FlintJVM/VM/Src/flint_method_info.cpp"
    SRCS "../../ESPJVM/FlintJVM/VM/Src/flint_object.cpp"
    SRCS "../../ESPJVM/FlintJVM/VM/Src/flint_out_of_memory.cpp"
    SRCS "../../ESPJVM/FlintJVM/VM/Src/flint_thread.cpp"
    SRCS "../../ESPJVM/FlintJVM/VM/Src/flint_string.cpp"
    SRCS "../../ESPJVM/FlintJVM/VM/Src/flint_throwable.cpp"
    SRCS "../../ESPJVM/FlintJVM/VM/Src/flint.cpp"
    SRCS "../../ESPJVM/FlintJVM/Native/Src/flint_native_character_class.cpp"
    SRCS "../../ESPJVM/FlintJVM/Native/Src/flint_native_class_class.cpp"
    SRCS "../../ESPJVM/FlintJVM/Native/Src/flint_native_class.cpp"
    SRCS "../../ESPJVM/FlintJVM/Native/Src/flint_native_double_class.cpp"
    SRCS "../../ESPJVM/FlintJVM/Native/Src/flint_native_float_class.cpp"
    SRCS "../../ESPJVM/FlintJVM/Native/Src/flint_native_math_class.cpp"
    SRCS "../../ESPJVM/FlintJVM/Native/Src/flint_native_object_class.cpp"
    SRCS "../../ESPJVM/FlintJVM/Native/Src/flint_native_thread_class.cpp"
    SRCS "../../ESPJVM/FlintJVM/Native/Src/flint_native_print_stream_class.cpp"
    SRCS "../../ESPJVM/FlintJVM/Native/Src/flint_native_string_class.cpp"
    SRCS "../../ESPJVM/FlintJVM/Native/Src/flint_native_system_class.cpp"

    INCLUDE_DIRS "../../ESPJVM/User/Inc"
    INCLUDE_DIRS "../../ESPJVM/Native/Inc"
    INCLUDE_DIRS "../../ESPJVM/SystemAPI/Inc"
    INCLUDE_DIRS "../../ESPJVM/FlintJVM/VM/Inc"
    INCLUDE_DIRS "../../ESPJVM/FlintJVM/Native/Inc"

    PRIV_REQUIRES "${priv_requires}"
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-address)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-volatile)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-uninitialized)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-strict-aliasing)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-implicit-fallthrough)
