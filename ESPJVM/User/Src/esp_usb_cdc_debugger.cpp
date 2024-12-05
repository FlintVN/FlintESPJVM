
#include <iostream>
#include "tusb.h"
#include "tusb_cdc_acm.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_debugger.h"

static SemaphoreHandle_t cdcRxSemaphore = NULL;

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

static void cdcRxCallback(int itf, cdcacm_event_t *event) {
    xSemaphoreGiveFromISR(cdcRxSemaphore, NULL);
}

void EspDebugger::receiveTask(void) {
    static uint8_t rxData[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];
    uint32_t rxDataToltalLength = 0;
    uint32_t rxDataLengthReceived = 0;
    TickType_t startTick = xTaskGetTickCount();

    if(cdcRxSemaphore == NULL)
        cdcRxSemaphore = xSemaphoreCreateBinary();

    tinyusb_cdcacm_register_callback(TINYUSB_CDC_ACM_0, CDC_EVENT_RX, &cdcRxCallback);

    while(1) {
        if(xSemaphoreTake(cdcRxSemaphore, portMAX_DELAY) == pdTRUE) {
            TickType_t tick = xTaskGetTickCount();
            if((tick - startTick) >= pdMS_TO_TICKS(100)) {
                rxDataToltalLength = 0;
                rxDataLengthReceived = 0;
            }
            uint32_t rxSize = tud_cdc_n_read(TINYUSB_CDC_ACM_0, &rxData[rxDataLengthReceived], sizeof(rxData) - rxDataLengthReceived);
            if(rxSize > 0) {
                rxDataLengthReceived += rxSize;
                if(rxDataToltalLength == 0 && rxDataLengthReceived >= 4)
                    rxDataToltalLength = rxData[1] | (rxData[2] << 8) | (rxData[3] << 16);
                if(rxDataToltalLength && (rxDataLengthReceived >= rxDataToltalLength) && espDbgInstance) {
                    espDbgInstance->receivedDataHandler(rxData, rxDataLengthReceived);
                    rxDataToltalLength = 0;
                    rxDataLengthReceived = 0;
                }
            }
            startTick = tick;
        }
    }
}

EspDebugger::~EspDebugger(void) {

}
