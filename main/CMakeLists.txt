set(priv_requires fatfs console esp_timer nvs_flash)

if(CONFIG_EXAMPLE_STORAGE_MEDIA_SPIFLASH)
    list(APPEND priv_requires wear_levelling esp_partition)
endif()

idf_component_register(
    SRCS "User/Src/main.cpp"
    SRCS "User/Src/usb_device.c"
    SRCS "User/Src/esp_uart_debugger.cpp"
    SRCS "User/Src/esp_system_api.cpp"
    SRCS "User/Src/esp_system_file_api.cpp"
    SRCS "User/Src/esp_system_thread_api.cpp"
    SRCS "User/Src/esp_system_native.cpp"

    SRCS "User/Src/esp_system_native_pin.cpp"

    SRCS "FlintJVM/VM/Src/flint_attribute_info.cpp"
    SRCS "FlintJVM/VM/Src/flint_class_loader.cpp"
    SRCS "FlintJVM/VM/Src/flint_class.cpp"
    SRCS "FlintJVM/VM/Src/flint_common.cpp"
    SRCS "FlintJVM/VM/Src/flint_const_name.cpp"
    SRCS "FlintJVM/VM/Src/flint_const_pool.cpp"
    SRCS "FlintJVM/VM/Src/flint_debugger.cpp"
    SRCS "FlintJVM/VM/Src/flint_execution.cpp"
    SRCS "FlintJVM/VM/Src/flint_field_info.cpp"
    SRCS "FlintJVM/VM/Src/flint_fields_data.cpp"
    SRCS "FlintJVM/VM/Src/flint_load_file_error.cpp"
    SRCS "FlintJVM/VM/Src/flint_method_info.cpp"
    SRCS "FlintJVM/VM/Src/flint_object.cpp"
    SRCS "FlintJVM/VM/Src/flint_out_of_memory.cpp"
    SRCS "FlintJVM/VM/Src/flint_thread.cpp"
    SRCS "FlintJVM/VM/Src/flint_string.cpp"
    SRCS "FlintJVM/VM/Src/flint_throwable.cpp"
    SRCS "FlintJVM/VM/Src/flint.cpp"
    SRCS "FlintJVM/Native/Src/flint_native_character_class.cpp"
    SRCS "FlintJVM/Native/Src/flint_native_class_class.cpp"
    SRCS "FlintJVM/Native/Src/flint_native_class.cpp"
    SRCS "FlintJVM/Native/Src/flint_native_double_class.cpp"
    SRCS "FlintJVM/Native/Src/flint_native_float_class.cpp"
    SRCS "FlintJVM/Native/Src/flint_native_math_class.cpp"
    SRCS "FlintJVM/Native/Src/flint_native_object_class.cpp"
    SRCS "FlintJVM/Native/Src/flint_native_thread_class.cpp"
    SRCS "FlintJVM/Native/Src/flint_native_print_stream_class.cpp"
    SRCS "FlintJVM/Native/Src/flint_native_string_class.cpp"
    SRCS "FlintJVM/Native/Src/flint_native_system_class.cpp"

    INCLUDE_DIRS "User/Inc"
    INCLUDE_DIRS "FlintJVM/VM/Inc"
    INCLUDE_DIRS "FlintJVM/Native/Inc"

    PRIV_REQUIRES "${priv_requires}"
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-address)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-volatile)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-uninitialized)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-strict-aliasing)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-implicit-fallthrough)
