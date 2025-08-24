
#include <string.h>
#include "flint_method_info.h"
#include "flint_system_api.h"
#include "flint_class_loader.h"
#include "flint_native.h"
#include "esp_system_native_pin.h"
#include "esp_system_native_spi.h"
#include "esp_system_native_port.h"
#include "esp_system_native_wifi.h"

static constexpr NativeClass ESP_NATIVE_CLASS_LIST[] = {
    NATIVE_CLASS("esp/machine/Pin",       pinMethods),
    NATIVE_CLASS("esp/machine/Port",      portMethods),
    NATIVE_CLASS("esp/machine/SpiMaster", spiMethods),
    NATIVE_CLASS("esp/network/WiFi",      wifiMethods),
};

void FlintAPI::System::reset(void) {
    NativePin_Reset();
    NativeSPI_Reset();
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
