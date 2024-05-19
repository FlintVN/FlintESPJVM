
#include <stdio.h>
#include "mjvm_system_api.h"

void MjvmSystem_Write(const char *text, uint32_t length, uint8_t coder) {
    char buff[64];
    if(coder == 0) {
        while(length) {
            uint32_t tmp = length < sizeof(buff) ? length : (sizeof(buff) - 1);
            length -= tmp;
            for(uint32_t i = 0; i < tmp; i++)
                buff[i] = text[i];
            text += tmp;
            buff[tmp] = 0;
            fprintf(stdout, buff);
        }
    }
    else {
        // TODO
    }
}

int64_t MjvmSystem_GetNanoTime(void) {
    throw "MjvmSystem_GetNanoTime is not implemented in VM";
}
