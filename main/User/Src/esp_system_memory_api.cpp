
#include <stdio.h>
#include "esp_heap_caps.h"
#include "flint_system_api.h"

void *FlintSystem_Malloc(uint32_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
}

void *FlintSystem_Realloc(void *p, uint32_t size) {
    return heap_caps_realloc(p, size, MALLOC_CAP_SPIRAM);
}

void FlintSystem_Free(void *p) {
    heap_caps_free(p);
}
