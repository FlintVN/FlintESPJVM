
#include <stdio.h>
#include "esp_timer.h"
#include "mjvm_string.h"
#include "mjvm_system_api.h"

void MjvmSystem_Write(const char *text, uint32_t length, uint8_t coder) {
    char buff[64];
    if(coder == 0) {
        while(length) {
            uint32_t count = 0;
            while(length && ((count + 2) <= sizeof(buff))) {
                count += MjvmString::utf8Encode((uint8_t)*text, &buff[count]);
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
                count += MjvmString::utf8Encode(*(uint16_t *)text, &buff[count]);
                text += 2;
                length--;
            }
            fprintf(stdout, "%*.*s", (int)count, (int)count, buff);
        }
    }
}

int64_t MjvmSystem_GetNanoTime(void) {
    return esp_timer_get_time();
}
