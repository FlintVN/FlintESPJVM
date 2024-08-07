
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "flint_system_api.h"

static int32_t lockNest = 0;
static volatile TaskHandle_t lockThreadId = 0;
static SemaphoreHandle_t semaphore = xSemaphoreCreateMutex();

void FlintAPI::Thread::lock(void) {
    TaskHandle_t currentThreadId = xTaskGetCurrentTaskHandle();
    while(1) {
        while(xSemaphoreTake(semaphore, 100) != pdTRUE);
        if(lockThreadId == 0) {
            lockNest = 1;
            lockThreadId = currentThreadId;
            xSemaphoreGive(semaphore);
            return;
        }
        else if(lockThreadId == currentThreadId) {
            lockNest++;
            xSemaphoreGive(semaphore);
            return;
        }
        xSemaphoreGive(semaphore);
        FlintAPI::Thread::yield();
    }
}

void FlintAPI::Thread::unlock(void) {
    while(xSemaphoreTake(semaphore, 100) != pdTRUE);
    if(--lockNest == 0)
        lockThreadId = 0;
    xSemaphoreGive(semaphore);
}

void *FlintAPI::Thread::create(void (*task)(void *), void *param, uint32_t stackSize) {
    TaskHandle_t xHandle = NULL;
    xTaskCreate(task, "FlintThread", stackSize ? stackSize : 4096, param, tskIDLE_PRIORITY + 1, &xHandle);
    return (void *)xHandle;
}

void FlintAPI::Thread::terminate(void *threadHandle) {
    vTaskDelete((TaskHandle_t)threadHandle);
}

void FlintAPI::Thread::sleep(uint32_t ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

void FlintAPI::Thread::yield(void) {
    taskYIELD();
}
