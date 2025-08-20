
#include <stdio.h>
#include "sdkconfig.h"
#include "esp_timer.h"
#include "flint_java_string.h"
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

bool FlintAPI::System::isInHeapRegion(void *addr) {
    return find_containing_heap(addr) != NULL_PTR;
}

void FlintAPI::System::print(const char *text, uint32_t length, uint8_t coder) {
    char buff[64];
    if(coder == 0) {
        while(length) {
            uint32_t count = 0;
            while(length && ((count + 2) <= sizeof(buff))) {
                count += FlintJavaString::utf8Encode((uint8_t)*text, &buff[count]);
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
                count += FlintJavaString::utf8Encode(*(uint16_t *)text, &buff[count]);
                text += 2;
                length--;
            }
            fprintf(stdout, "%*.*s", (int)count, (int)count, buff);
        }
    }
}

uint64_t FlintAPI::System::getNanoTime(void) {
    return (uint64_t)esp_timer_get_time() * 1000;
}
