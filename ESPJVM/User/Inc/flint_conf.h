
#ifndef __FLINT_CONF_H
#define __FLINT_CONF_H

#include "sdkconfig.h"
#include "flint_common.h"

#if CONFIG_IDF_TARGET_ESP32
    #define FLINT_VARIANT_NAME      "ESP32 FlintJVM"
#elif CONFIG_IDF_TARGET_ESP32C2
    #error "Not supported for ESP32C2"
#elif CONFIG_IDF_TARGET_ESP32C3
    #error "Not supported for ESP32C3"
#elif CONFIG_IDF_TARGET_ESP32C6
    #error "Not supported for ESP32C6"
#elif CONFIG_IDF_TARGET_ESP32C5
    #error "Not supported for ESP32C5"
#elif CONFIG_IDF_TARGET_ESP32H2
    #error "Not supported for ESP32H2"
#elif CONFIG_IDF_TARGET_ESP32P4
    #error "Not supported for ESP32P4"
#elif CONFIG_IDF_TARGET_ESP32S2
    #define FLINT_VARIANT_NAME      "ESP32S2 FlintJVM"
#elif CONFIG_IDF_TARGET_ESP32S3
    #define FLINT_VARIANT_NAME      "ESP32S3 FlintJVM"
#else
    #error "Unknown ESP32 target"
#endif

#define FILE_NAME_BUFF_SIZE         256

#define DEFAULT_STACK_SIZE          KILO_BYTE(10)
#define OBJECT_SIZE_TO_GC           KILO_BYTE(10)

#define MAX_OF_BREAK_POINT          20
#define DBG_TX_BUFFER_SIZE          512
#define DBG_CONSOLE_BUFFER_SIZE     KILO_BYTE(1)

#endif /* __FLINT_CONF_H */
