
#include <iostream>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/uart.h"
#include "esp_debugger.h"

EspDebugger *EspDebugger::espDbgInstance = NULL_PTR;

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
        size_t byteSent = uart_write_bytes(UART_NUM_0, data, length);
        if(byteSent == 0)
            return false;
        length -= byteSent;
        data += byteSent;
    }
    return true;
}

void EspDebugger::receiveTask(void) {
    static uint8_t rxData[1024 + 1];
    static QueueHandle_t uartQueue;
    uint32_t rxDataToltalLength = 0;
    uint32_t rxDataLengthReceived = 0;
    uart_event_t event;
    TickType_t startTick = xTaskGetTickCount();

    uart_driver_install(UART_NUM_0, 1024 * 2, 0, 20, &uartQueue, 0);

    while(1) {
        if(xQueueReceive(uartQueue, (void *)&event, portMAX_DELAY)) {
            TickType_t tick = xTaskGetTickCount();
            if((tick - startTick) >= pdMS_TO_TICKS(100)) {
                rxDataToltalLength = 0;
                rxDataLengthReceived = 0;
            }
            uint32_t rxSize = uart_read_bytes(UART_NUM_0, &rxData[rxDataLengthReceived], sizeof(rxData) - rxDataLengthReceived, 0);
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
