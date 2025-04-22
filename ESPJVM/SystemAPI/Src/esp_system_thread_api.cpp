
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "flint_system_api.h"

FlintAPI::Thread::LockHandle *FlintAPI::Thread::createLockHandle(void) {
    FlintAPI::Thread::LockHandle *lockHandle;
    lockHandle = (FlintAPI::Thread::LockHandle *)FlintAPI::System::malloc(sizeof(FlintAPI::Thread::LockHandle));
    if(!lockHandle)
        return NULL;
    lockHandle->mutexHandle = (void *)xSemaphoreCreateMutex();
    lockHandle->lockThreadId = 0;
    lockHandle->lockNest = 0;
    return lockHandle;
}

void FlintAPI::Thread::lock(FlintAPI::Thread::LockHandle *lockHandle) {
    TaskHandle_t currentThreadId = xTaskGetCurrentTaskHandle();
    while(1) {
        while(xSemaphoreTake((SemaphoreHandle_t)lockHandle->mutexHandle, 100) != pdTRUE);
        if(lockHandle->lockThreadId == 0) {
            lockHandle->lockNest = 1;
            lockHandle->lockThreadId = currentThreadId;
            xSemaphoreGive((SemaphoreHandle_t)lockHandle->mutexHandle);
            return;
        }
        else if(lockHandle->lockThreadId == currentThreadId) {
            lockHandle->lockNest++;
            xSemaphoreGive((SemaphoreHandle_t)lockHandle->mutexHandle);
            return;
        }
        xSemaphoreGive((SemaphoreHandle_t)lockHandle->mutexHandle);
        FlintAPI::Thread::yield();
    }
}

void FlintAPI::Thread::unlock(FlintAPI::Thread::LockHandle *lockHandle) {
    while(xSemaphoreTake((SemaphoreHandle_t)lockHandle->mutexHandle, 100) != pdTRUE);
    if(--lockHandle->lockNest == 0)
        lockHandle->lockThreadId = 0;
    xSemaphoreGive((SemaphoreHandle_t)lockHandle->mutexHandle);
}

void *FlintAPI::Thread::create(void (*task)(void *), void *param, uint32_t stackSize) {
    TaskHandle_t xHandle = NULL;
    xTaskCreate(task, "FlintJavaThread", stackSize ? stackSize : 5120, param, tskIDLE_PRIORITY + 1, &xHandle);
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
