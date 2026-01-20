
#include <string.h>
#include "flint_method_info.h"
#include "flint_system_api.h"
#include "flint_class_loader.h"
#include "flint_native.h"
#include "esp_socket.h"
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
#include "esp_native_flint_socket_impl.h"
#include "esp_native_flint_inet_address_impl.h"
#include "esp_native_flint_socket_input_Stream.h"
#include "esp_native_flint_socket_output_Stream.h"
#include "esp_native_flint_datagram_socket_impl.h"

static constexpr NativeClass ESP_NATIVE_CLASS_LIST[] = {
    NATIVE_CLASS("flint/net/WiFi",                    wifiMethods),
    NATIVE_CLASS("flint/io/Adc",                      adcMethods),
    NATIVE_CLASS("flint/io/Dac",                      dacMethods),
    NATIVE_CLASS("flint/io/Pin",                      pinMethods),
    NATIVE_CLASS("flint/io/Port",                     portMethods),
    NATIVE_CLASS("flint/io/SerialPort",               uartMethods),
    NATIVE_CLASS("flint/io/OneWire",                  oneWireMethods),
    NATIVE_CLASS("flint/io/SpiMaster",                spiMasterMethods),
    NATIVE_CLASS("flint/io/I2cMaster",                i2cMasterMethods),
    NATIVE_CLASS("flint/io/I2sMaster",                i2sMasterMethods),
    NATIVE_CLASS("flint/io/BitStream",                bitStreamMethods),
    NATIVE_CLASS("flint/net/FlintSocketImpl",         flintSocketImplMethods),
    NATIVE_CLASS("flint/net/FlintInetAddressImpl",    flintInetAddressImplMethods),
    NATIVE_CLASS("flint/net/FlintSocketInputStream",  flintSocketInputStreamMethods),
    NATIVE_CLASS("flint/net/FlintSocketOutputStream", flintSocketOutputStreamMethods),
    NATIVE_CLASS("flint/net/FlintDatagramSocketImpl", flintDatagramSocketImplMethods),
};

void FlintAPI::System::reset(void) {
    NativeAdc_Reset();
    NativeDac_Reset();
    NativePin_Reset();
    NativeUart_Reset();
    NativeSpiMaster_Reset();
    NativeI2cMaster_Reset();
    NativeI2sMaster_Reset();
    Socket_Reset();
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
