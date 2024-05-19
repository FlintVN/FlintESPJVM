
#include <stdio.h>
#include "esp_heap_caps.h"
#include "mjvm_system_api.h"

void *MjvmSystem_Malloc(uint32_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
}

void MjvmSystem_Free(void *p) {
    heap_caps_free(p);
}
