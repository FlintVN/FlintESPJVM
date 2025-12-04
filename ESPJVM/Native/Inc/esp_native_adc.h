

#ifndef __ESP_NATIVE_ADC_H
#define __ESP_NATIVE_ADC_H

#include "flint_native.h"

void NativeAdc_Reset(void);

jvoid NativeAdc_InitAdc(FNIEnv *env, jobject obj);
jint NativeAdc_Read(FNIEnv *env, jobject obj);

static constexpr NativeMethod adcMethods[] = {
    NATIVE_METHOD("initAdc", "()V", NativeAdc_InitAdc),
    NATIVE_METHOD("read",    "()I", NativeAdc_Read),
};

#endif /* __ESP_NATIVE_ADC_H */
