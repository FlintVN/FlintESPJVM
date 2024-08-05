
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "flint_system_api.h"

void *FlintAPI::Thread::create(void (*task)(void *), void *param, uint32_t stackSize) {
    TaskHandle_t xHandle = NULL;
    xTaskCreate(task, "FlintThread", stackSize ? stackSize : 4096, param, tskIDLE_PRIORITY + 1, &xHandle);
    return (void *)xHandle;
}

void FlintAPI::Thread::terminate(void *threadHandle) {
    vTaskDelete((TaskHandle_t)threadHandle);
}

void FlintAPI::Thread::sleep(uint32_t ms) {
    vTaskDelay(ms);
}
