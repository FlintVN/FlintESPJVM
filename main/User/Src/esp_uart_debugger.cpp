
#include <iostream>
#include "tusb.h"
#include "tusb_cdc_acm.h"
#include "esp_uart_debugger.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "driver/gpio.h"

static EspUartDebugger *espUartDbg = 0;

EspUartDebugger::EspUartDebugger(Flint &flint) : FlintDebugger(flint) {

}

EspUartDebugger &EspUartDebugger::getInstance(Flint &flint) {
    if(espUartDbg == 0) {
        espUartDbg = (EspUartDebugger *)Flint::malloc(sizeof(EspUartDebugger));
        new (espUartDbg)EspUartDebugger(flint);
    }
    return *espUartDbg;
}

bool EspUartDebugger::sendData(uint8_t *data, uint32_t length) {
    while(length) {
        size_t queueSize = tinyusb_cdcacm_write_queue(TINYUSB_CDC_ACM_0, data, length);
        if(tinyusb_cdcacm_write_flush(TINYUSB_CDC_ACM_0, 0) != ESP_OK)
            return false;
        length -= queueSize;
        data += queueSize;
    }
    return true;
}

void EspUartDebugger::receiveTask(void) {
    static uint8_t rxData[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];
    uint32_t rxDataLength = 0;
    uint32_t rxDataLengthReceived = 0;
    TickType_t startTick = 0;

    while(1) {
        if(tud_cdc_n_available(TINYUSB_CDC_ACM_0)) {
            TickType_t tick = xTaskGetTickCount();
            if((rxDataLength == 0) || ((tick - startTick) >= pdMS_TO_TICKS(100))) {
                uint32_t rxSize = tud_cdc_n_read(TINYUSB_CDC_ACM_0, rxData, sizeof(rxData));
                rxDataLength = rxData[1] | (rxData[2] << 8) | (rxData[3] << 16);
                rxDataLengthReceived = rxSize;
            }
            else {
                uint32_t rxSize = tud_cdc_n_read(TINYUSB_CDC_ACM_0, &rxData[rxDataLengthReceived], sizeof(rxData) - rxDataLengthReceived);
                rxDataLengthReceived += rxSize;
            }
            if(rxDataLength && (rxDataLengthReceived >= rxDataLength) && espUartDbg) {
                espUartDbg->receivedDataHandler(rxData, rxDataLengthReceived);
                rxDataLength = 0;
                rxDataLengthReceived = 0;
            }
        }
        vTaskDelay(1);
    }
}

EspUartDebugger::~EspUartDebugger(void) {

}
