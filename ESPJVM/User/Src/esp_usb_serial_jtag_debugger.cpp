
#include <iostream>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/usb_serial_jtag.h"
#include "esp_debugger.h"

EspDebugger *EspDebugger::espDbgInstance = 0;

EspDebugger::EspDebugger(Flint &flint) : FlintDebugger(flint) {

}

EspDebugger &EspDebugger::getInstance(Flint &flint) {
    if(espDbgInstance == 0) {
        espDbgInstance = (EspDebugger *)Flint::malloc(sizeof(EspDebugger));
        new (espDbgInstance)EspDebugger(flint);
    }
    return *espDbgInstance;
}

bool EspDebugger::sendData(uint8_t *data, uint32_t length) {
    while(length) {
        size_t byteSent = usb_serial_jtag_write_bytes((const char *)data, length, 20 / portTICK_PERIOD_MS);
        if(byteSent == 0)
            return false;
        length -= byteSent;
        data += byteSent;
    }
    return true;
}

void EspDebugger::receiveTask(void) {
    static uint8_t rxData[1024 + 1];
    uint32_t rxDataToltalLength = 0;
    uint32_t rxDataLengthReceived = 0;
    TickType_t startTick = xTaskGetTickCount();

    while(1) {
        uint32_t rxSize = usb_serial_jtag_read_bytes(&rxData[rxDataLengthReceived], sizeof(rxData) - rxDataLengthReceived, portMAX_DELAY);
        if(rxSize > 0) {
            TickType_t tick = xTaskGetTickCount();
            if((tick - startTick) >= pdMS_TO_TICKS(100)) {
                rxDataToltalLength = 0;
                rxDataLengthReceived = 0;
            }
            rxDataLengthReceived += rxSize;
            if(rxDataToltalLength == 0 && rxDataLengthReceived >= 4)
                rxDataToltalLength = rxData[1] | (rxData[2] << 8) | (rxData[3] << 16);
            if(rxDataToltalLength && (rxDataLengthReceived >= rxDataToltalLength) && espDbgInstance) {
                espDbgInstance->receivedDataHandler(rxData, rxDataLengthReceived);
                rxDataToltalLength = 0;
                rxDataLengthReceived = 0;
            }
            startTick = tick;
        }
    }
}

EspDebugger::~EspDebugger(void) {

}
