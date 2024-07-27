
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "flint_system_api.h"

void *FlintSystem_ThreadCreate(void (*task)(void *), void *param, uint32_t stackSize) {
    TaskHandle_t xHandle = NULL;
    xTaskCreate(task, "FlintThread", stackSize ? stackSize : 10240, param, tskIDLE_PRIORITY, &xHandle);
    return (void *)xHandle;
}

void FlintSystem_ThreadTerminate(void *threadHandle) {
    vTaskDelete((TaskHandle_t)threadHandle);
}

void FlintSystem_ThreadSleep(uint32_t ms) {
    vTaskDelay(ms);
}
