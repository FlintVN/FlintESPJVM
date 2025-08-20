
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "flint_system_api.h"

void *FlintAPI::Thread::create(void (*task)(void *), void *param, uint32_t stackSize) {
    TaskHandle_t xHandle = NULL_PTR;
    xTaskCreate(task, "FlintJavaThread", stackSize ? stackSize : 5120, param, tskIDLE_PRIORITY + 1, &xHandle);
    return (void *)xHandle;
}

void *FlintAPI::Thread::getCurrentThread(void) {
    return (void *)xTaskGetCurrentTaskHandle();
}

void FlintAPI::Thread::terminate(void *threadHandle) {
    vTaskDelete((TaskHandle_t)threadHandle);
}

void FlintAPI::Thread::sleep(uint32_t ms) {
    TickType_t tick = pdMS_TO_TICKS(ms);
    vTaskDelay((tick < 1) ? 1 : tick);
}

void FlintAPI::Thread::yield(void) {
    taskYIELD();
}
