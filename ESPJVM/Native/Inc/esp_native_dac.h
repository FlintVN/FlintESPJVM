

#ifndef __ESP_NATIVE_DAC_H
#define __ESP_NATIVE_DAC_H

#include "flint_native.h"

void NativeDac_Reset(void);

jvoid NativeDac_InitDac(FNIEnv *env, jobject obj);
jvoid NativeDac_Write(FNIEnv *env, jobject obj, jint value);

static constexpr NativeMethod dacMethods[] = {
    NATIVE_METHOD("initDac", "()V",  NativeDac_InitDac),
    NATIVE_METHOD("write",   "(I)V", NativeDac_Write),
};

#endif /* __ESP_NATIVE_DAC_H */
