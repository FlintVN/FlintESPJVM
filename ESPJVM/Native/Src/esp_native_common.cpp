
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_native_common.h"

static portMUX_TYPE atomicMux = portMUX_INITIALIZER_UNLOCKED;

void EnterAtomicSection(void) {
    portENTER_CRITICAL(&atomicMux);
}

void ExitAtomicSection(void) {
    portEXIT_CRITICAL(&atomicMux);
}
