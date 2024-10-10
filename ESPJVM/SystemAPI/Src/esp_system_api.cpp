
#include <stdio.h>
#include "sdkconfig.h"
#include "esp_timer.h"
#include "flint_string.h"
#include "esp_heap_caps.h"
#include "flint_system_api.h"

void *FlintAPI::System::malloc(uint32_t size) {
#if CONFIG_SPIRAM
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
#else
    return ::malloc(size);
#endif
}

void *FlintAPI::System::realloc(void *p, uint32_t size) {
#if CONFIG_SPIRAM
    return heap_caps_realloc(p, size, MALLOC_CAP_SPIRAM);
#else
    return ::realloc(p, size);
#endif
}

void FlintAPI::System::free(void *p) {
#if CONFIG_SPIRAM
    heap_caps_free(p);
#else
    ::free(p);
#endif
}

void FlintAPI::System::print(const char *text, uint32_t length, uint8_t coder) {
    char buff[64];
    if(coder == 0) {
        while(length) {
            uint32_t count = 0;
            while(length && ((count + 2) <= sizeof(buff))) {
                count += FlintString::utf8Encode((uint8_t)*text, &buff[count]);
                text++;
                length--;
            }
            fprintf(stdout, "%*.*s", (int)count, (int)count, buff);
        }
    }
    else {
        while(length) {
            uint32_t count = 0;
            while(length && ((count + 3) <= sizeof(buff))) {
                count += FlintString::utf8Encode(*(uint16_t *)text, &buff[count]);
                text += 2;
                length--;
            }
            fprintf(stdout, "%*.*s", (int)count, (int)count, buff);
        }
    }
}

int64_t FlintAPI::System::getNanoTime(void) {
    return esp_timer_get_time();
}
