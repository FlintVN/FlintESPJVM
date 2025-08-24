set(priv_requires fatfs console esp_timer nvs_flash driver wear_levelling esp_partition esp_wifi)

idf_component_register(
    SRCS "../../../ESPJVM/User/Src/main.cpp"
    SRCS "../../../ESPJVM/User/Src/esp_board_esp32.cpp"
    SRCS "../../../ESPJVM/User/Src/esp_uart_debugger.cpp"

    SRCS "../../../ESPJVM/SystemAPI/Src/esp_system_api.cpp"
    SRCS "../../../ESPJVM/SystemAPI/Src/esp_system_io_api.cpp"
    SRCS "../../../ESPJVM/SystemAPI/Src/esp_system_thread_api.cpp"

    SRCS "../../../ESPJVM/Native/Src/esp_system_native.cpp"
    SRCS "../../../ESPJVM/Native/Src/esp_system_native_pin.cpp"
    SRCS "../../../ESPJVM/Native/Src/esp_system_native_spi.cpp"
    SRCS "../../../ESPJVM/Native/Src/esp_system_native_port.cpp"
    SRCS "../../../ESPJVM/Native/Src/esp_system_native_wifi.cpp"

    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_utf8.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_mutex.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_common.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_const_pool.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_fields_data.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_field_info.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_method_info.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_java_class.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_java_object.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_java_string.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_java_thread.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_java_throwable.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_array_object.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_class_loader.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_execution.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_debugger.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_utf8_dict_node.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_java_class_dict_node.cpp"
    SRCS "../../../ESPJVM/FlintJVM/VM/Src/flint_java_string_dict_node.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_interface.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_math.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_class.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_array.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_float.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_double.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_object.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_string.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_system.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_thread.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_character.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_reflection.cpp"
    SRCS "../../../ESPJVM/FlintJVM/Native/Src/flint_native_print_stream.cpp"

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
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-cast-function-type)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-ignored-qualifiers)
