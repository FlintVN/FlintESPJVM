
#include <string.h>
#include <iostream>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/usb_serial_jtag.h"
#include "flint_system_api.h"
#include "esp_debugger.h"

EspDbg *EspDbg::espDbgInstance = NULL;

EspDbg::EspDbg(void) : FDbg() {

}

EspDbg *EspDbg::getInstance(void) {
    if(espDbgInstance == 0) {
        espDbgInstance = (EspDbg *)FlintAPI::System::malloc(sizeof(EspDbg));
        new (espDbgInstance)EspDbg();
    }
    return espDbgInstance;
}

bool EspDbg::sendData(uint8_t *data, uint32_t length) {
    while(length) {
        size_t byteSent = usb_serial_jtag_write_bytes(data, length, 20 / portTICK_PERIOD_MS);
        if(byteSent == 0)
            return false;
        length -= byteSent;
        data += byteSent;
    }
    return true;
}

void EspDbg::receiveTask(void) {
    static uint8_t rxData[1024 + 1];
    uint32_t rxDataToltalLength = 0;
    uint32_t rxDataLengthReceived = 0;
    TickType_t startTick = xTaskGetTickCount();

    while(1) {
        uint32_t rxSize = usb_serial_jtag_read_bytes(&rxData[rxDataLengthReceived], sizeof(rxData) - rxDataLengthReceived, portMAX_DELAY);
        if(rxSize > 0) {
            TickType_t tick = xTaskGetTickCount();
            if(rxDataLengthReceived > 0 && (tick - startTick) >= pdMS_TO_TICKS(100)) {
                memcpy(rxData, &rxData[rxDataLengthReceived], rxSize);
                rxDataLengthReceived = 0;
                rxDataToltalLength = 0;
            }
            if(rxDataLengthReceived == 0) {
                if(rxData[0] == 0)
                    rxDataLengthReceived += rxSize;
            }
            else
                rxDataLengthReceived += rxSize;
            if(rxDataToltalLength == 0 && rxDataLengthReceived >= 4)
                rxDataToltalLength = (rxData[1] >> 6) | (rxData[2] << 2) | (rxData[3] << 10);
            if(rxDataToltalLength && (rxDataLengthReceived >= rxDataToltalLength) && espDbgInstance) {
                espDbgInstance->receivedDataHandler(rxData, rxDataLengthReceived);
                rxDataToltalLength = 0;
                rxDataLengthReceived = 0;
            }
            startTick = xTaskGetTickCount();
        }
    }
}

EspDbg::~EspDbg(void) {

}
