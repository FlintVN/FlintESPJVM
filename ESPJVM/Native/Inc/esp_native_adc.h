

#ifndef __ESP_NATIVE_ADC_H
#define __ESP_NATIVE_ADC_H

#include "flint_native.h"

void NativeAdc_Reset(void);

jvoid nativeAdcInitAdc(FNIEnv *env, jobject obj);
jint nativeAdcRead(FNIEnv *env, jobject obj);

static constexpr NativeMethod adcMethods[] = {
    NATIVE_METHOD("initAdc", "()V", nativeAdcInitAdc),
    NATIVE_METHOD("read",    "()I", nativeAdcRead),
};

#endif /* __ESP_NATIVE_ADC_H */
