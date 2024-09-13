
#include <iostream>
#include "tusb.h"
#include "tusb_cdc_acm.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
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
        size_t queueSize = tinyusb_cdcacm_write_queue(TINYUSB_CDC_ACM_0, data, length);
        if(tinyusb_cdcacm_write_flush(TINYUSB_CDC_ACM_0, 0) != ESP_OK)
            return false;
        length -= queueSize;
        data += queueSize;
    }
    return true;
}

void EspDebugger::receiveTask(void) {
    static uint8_t rxData[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];
    uint32_t rxDataToltalLength = 0;
    uint32_t rxDataLengthReceived = 0;
    TickType_t startTick = 0;

    while(1) {
        if(tud_cdc_n_available(TINYUSB_CDC_ACM_0)) {
            TickType_t tick = xTaskGetTickCount();
            if((rxDataToltalLength == 0) || ((tick - startTick) >= pdMS_TO_TICKS(100))) {
                uint32_t rxSize = tud_cdc_n_read(TINYUSB_CDC_ACM_0, rxData, sizeof(rxData));
                rxDataToltalLength = rxData[1] | (rxData[2] << 8) | (rxData[3] << 16);
                rxDataLengthReceived = rxSize;
            }
            else {
                uint32_t rxSize = tud_cdc_n_read(TINYUSB_CDC_ACM_0, &rxData[rxDataLengthReceived], sizeof(rxData) - rxDataLengthReceived);
                rxDataLengthReceived += rxSize;
            }
            if(rxDataToltalLength && (rxDataLengthReceived >= rxDataToltalLength) && espDbgInstance) {
                espDbgInstance->receivedDataHandler(rxData, rxDataLengthReceived);
                rxDataToltalLength = 0;
                rxDataLengthReceived = 0;
            }
            startTick = tick;
        }
        vTaskDelay(1);
    }
}

EspDebugger::~EspDebugger(void) {

}
