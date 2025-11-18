
#include <string.h>
#include "flint_method_info.h"
#include "flint_system_api.h"
#include "flint_class_loader.h"
#include "flint_native.h"
#include "esp_system_native_pin.h"
#include "esp_system_native_spi_master.h"
#include "esp_system_native_uart.h"
#include "esp_system_native_port.h"
#include "esp_system_native_wifi.h"

static constexpr NativeClass ESP_NATIVE_CLASS_LIST[] = {
    NATIVE_CLASS("flint/machine/Pin",        pinMethods),
    NATIVE_CLASS("flint/machine/Port",       portMethods),
    NATIVE_CLASS("flint/machine/SerialPort", uartMethods),
    NATIVE_CLASS("flint/machine/SpiMaster",  spiMasterMethods),
    NATIVE_CLASS("flint/network/WiFi",       wifiMethods),
};

void FlintAPI::System::reset(void) {
    NativePin_Reset();
    NativeUart_Reset();
    NativeSpiMaster_Reset();
}

JNMPtr FlintAPI::System::findNativeMethod(MethodInfo *methodInfo) {
    uint32_t classNameHash = methodInfo->loader->getHashKey();
    for(uint32_t i = 0; i < LENGTH(ESP_NATIVE_CLASS_LIST); i++) {
        const NativeClass *nativeCls = &ESP_NATIVE_CLASS_LIST[i];
        if(
            classNameHash == nativeCls->hash &&
            strcmp(nativeCls->className, methodInfo->loader->getName()) == 0
        ) {
            for(uint32_t k = 0; k < nativeCls->methodCount; k++) {
                if(
                    nativeCls->methods[k].hash == methodInfo->hash &&
                    strcmp(nativeCls->methods[k].name, methodInfo->name) == 0 &&
                    strcmp(nativeCls->methods[k].desc, methodInfo->desc) == 0
                ) {
                    return (JNMPtr)nativeCls->methods[k].methodPtr;
                }
            }
            break;
        }
    }
    return NULL;
}
