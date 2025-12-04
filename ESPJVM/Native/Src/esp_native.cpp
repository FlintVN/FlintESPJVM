
#include <string.h>
#include "flint_method_info.h"
#include "flint_system_api.h"
#include "flint_class_loader.h"
#include "flint_native.h"
#include "esp_native_adc.h"
#include "esp_native_dac.h"
#include "esp_native_pin.h"
#include "esp_native_port.h"
#include "esp_native_wifi.h"
#include "esp_native_uart.h"
#include "esp_native_onewire.h"
#include "esp_native_bitstream.h"
#include "esp_native_spi_master.h"
#include "esp_native_i2c_master.h"
#include "esp_native_i2s_master.h"

static constexpr NativeClass ESP_NATIVE_CLASS_LIST[] = {
    NATIVE_CLASS("flint/machine/Adc",        adcMethods),
    NATIVE_CLASS("flint/machine/Dac",        dacMethods),
    NATIVE_CLASS("flint/machine/Pin",        pinMethods),
    NATIVE_CLASS("flint/machine/Port",       portMethods),
    NATIVE_CLASS("flint/network/WiFi",       wifiMethods),
    NATIVE_CLASS("flint/machine/SerialPort", uartMethods),
    NATIVE_CLASS("flint/machine/OneWire",    oneWireMethods),
    NATIVE_CLASS("flint/machine/SpiMaster",  spiMasterMethods),
    NATIVE_CLASS("flint/machine/I2cMaster",  i2cMasterMethods),
    NATIVE_CLASS("flint/machine/I2sMaster",  i2sMasterMethods),
    NATIVE_CLASS("flint/machine/BitStream",  bitStreamMethods),
};

void FlintAPI::System::reset(void) {
    NativeAdc_Reset();
    NativeDac_Reset();
    NativePin_Reset();
    NativeUart_Reset();
    NativeSpiMaster_Reset();
    NativeI2cMaster_Reset();
    NativeI2sMaster_Reset();
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
