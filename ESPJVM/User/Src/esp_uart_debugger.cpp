
#include <iostream>
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
    // TODO
    return true;
}

void EspDebugger::receiveTask(void) {
    // TODO
}

EspDebugger::~EspDebugger(void) {

}
