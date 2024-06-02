set(priv_requires fatfs console esp_timer nvs_flash)

if(CONFIG_EXAMPLE_STORAGE_MEDIA_SPIFLASH)
    list(APPEND priv_requires wear_levelling esp_partition)
endif()

idf_component_register(
    SRCS "User/Src/main.cpp"
    SRCS "User/Src/usb_device.c"
    SRCS "User/Src/esp_system_api.cpp"
    SRCS "User/Src/esp_system_file_api.cpp"
    SRCS "User/Src/esp_system_memory_api.cpp"
    INCLUDE_DIRS "User/Inc"

    SRCS "MJVM/MJVM/VM/Src/mjvm_attribute_info.cpp"
    SRCS "MJVM/MJVM/VM/Src/mjvm_class_loader.cpp"
    SRCS "MJVM/MJVM/VM/Src/mjvm_common.cpp"
    SRCS "MJVM/MJVM/VM/Src/mjvm_const_name.cpp"
    SRCS "MJVM/MJVM/VM/Src/mjvm_const_pool.cpp"
    SRCS "MJVM/MJVM/VM/Src/mjvm_execution.cpp"
    SRCS "MJVM/MJVM/VM/Src/mjvm_field_info.cpp"
    SRCS "MJVM/MJVM/VM/Src/mjvm_fields_data.cpp"
    SRCS "MJVM/MJVM/VM/Src/mjvm_method_info.cpp"
    SRCS "MJVM/MJVM/VM/Src/mjvm_object.cpp"
    SRCS "MJVM/MJVM/VM/Src/mjvm_string.cpp"
    SRCS "MJVM/MJVM/VM/Src/mjvm_throwable.cpp"
    SRCS "MJVM/MJVM/VM/Src/mjvm.cpp"

    SRCS "MJVM/MJVM/Native/Src/mjvm_native_character_class.cpp"
    SRCS "MJVM/MJVM/Native/Src/mjvm_native_class.cpp"
    SRCS "MJVM/MJVM/Native/Src/mjvm_native_double_class.cpp"
    SRCS "MJVM/MJVM/Native/Src/mjvm_native_float_class.cpp"
    SRCS "MJVM/MJVM/Native/Src/mjvm_native_math_class.cpp"
    SRCS "MJVM/MJVM/Native/Src/mjvm_native_object_class.cpp"
    SRCS "MJVM/MJVM/Native/Src/mjvm_native_print_stream_class.cpp"
    SRCS "MJVM/MJVM/Native/Src/mjvm_native_string_class.cpp"
    SRCS "MJVM/MJVM/Native/Src/mjvm_native_system_class.cpp"

    INCLUDE_DIRS "MJVM/MJVM/VM/Inc"
    INCLUDE_DIRS "MJVM/MJVM/Native/Inc"

    PRIV_REQUIRES "${priv_requires}"
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-address)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-uninitialized)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-implicit-fallthrough)
