
#include "flint_system_api.h"
#include "flint_class_loader.h"
#include "esp_system_native_pin.h"
#include "esp_system_native_spi.h"
#include "esp_system_native_port.h"
#include "esp_system_native_wifi.h"

static const FlintNativeClass *ESP_NATIVE_CLASS_LIST[] = {
    &PIN_CLASS,
    &SPI_CLASS,
    &PORT_CLASS,
    &WIFI_CLASS,
};

void FlintAPI::System::reset(Flint &flint) {
    NativePin_Reset(flint);
    NativeSPI_Reset(flint);
}

FlintNativeMethodPtr FlintAPI::System::findNativeMethod(FlintMethodInfo *methodInfo) {
    FlintConstUtf8 &className = *methodInfo->classLoader.thisClass;
    FlintConstUtf8 &methodName = methodInfo->getName();
    FlintConstUtf8 &methodDesc = methodInfo->getDescriptor();
    for(uint32_t i = 0; i < LENGTH(ESP_NATIVE_CLASS_LIST); i++) {
        if(ESP_NATIVE_CLASS_LIST[i]->className == className) {
            for(uint32_t k = 0; k < ESP_NATIVE_CLASS_LIST[i]->methodCount; k++) {
                if(
                    ESP_NATIVE_CLASS_LIST[i]->methods[k].name == methodName &&
                    ESP_NATIVE_CLASS_LIST[i]->methods[k].descriptor == methodDesc
                ) {
                    return ESP_NATIVE_CLASS_LIST[i]->methods[k].nativeMathod;
                }
            }
            break;
        }
    }
    return (FlintNativeMethodPtr)0;
}
