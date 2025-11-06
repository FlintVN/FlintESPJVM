
#include <stdio.h>
#include "sdkconfig.h"
#include "esp_timer.h"
#include "flint_utf8.h"
#include "esp_heap_caps.h"
#include "../heap_private.h"
#include "flint_system_api.h"

void *FlintAPI::System::malloc(uint32_t size) {
    return ::malloc(size);
}

void *FlintAPI::System::realloc(void *p, uint32_t size) {
    return ::realloc(p, size);
}

void FlintAPI::System::free(void *p) {
    ::free(p);
}

void FlintAPI::System::consoleWrite(uint8_t *utf8, uint32_t length) {
    fprintf(stdout, "%.*s", (int)length, (char *)utf8);
}

uint64_t FlintAPI::System::getNanoTime(void) {
    return (uint64_t)esp_timer_get_time() * 1000;
}
