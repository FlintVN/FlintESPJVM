# Automatically generate a single merged binary (bootloader + partition table + app + storage)
# after every build. Output: boards/build/<project_name>_Release.bin (flash it at offset 0x0).
# Include this file after project() and fatfs_create_spiflash_image().

idf_build_get_property(_merge_python PYTHON)
idf_build_get_property(_merge_idf_path IDF_PATH)
idf_build_get_property(_merge_target IDF_TARGET)

set(_merge_output_dir "${CMAKE_CURRENT_LIST_DIR}/build")
set(_merge_output "${_merge_output_dir}/${CMAKE_PROJECT_NAME}_Release.bin")

add_custom_target(merged_bin ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_merge_output_dir}"
    COMMAND ${_merge_python} "${_merge_idf_path}/components/esptool_py/esptool/esptool.py"
            --chip ${_merge_target} merge_bin -o "${_merge_output}" "@flash_args"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    DEPENDS gen_project_binary bootloader partition_table_bin fatfs_storage_bin
    BYPRODUCTS "${_merge_output}"
    COMMENT "Generating merged binary ${CMAKE_PROJECT_NAME}_Release.bin"
    VERBATIM
)
