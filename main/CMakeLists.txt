set(priv_requires fatfs console)

if(CONFIG_EXAMPLE_STORAGE_MEDIA_SPIFLASH)
    list(APPEND priv_requires wear_levelling esp_partition)
endif()

idf_component_register(
    SRCS "User/Src/main.cpp"
    SRCS "User/Src/usb_msc.c"
    INCLUDE_DIRS "User/Inc"

    PRIV_REQUIRES "${priv_requires}"
)
