

#ifndef __ESP_NATIVE_ADC_CONTINUOUS_H
#define __ESP_NATIVE_ADC_CONTINUOUS_H

#include "flint_native.h"

void NativeAdcContinuous_Reset(void);

jvoid NativeAdcContinuous_Start(FNIEnv *env, jobject obj);
jbool NativeAdcContinuous_IsStart(FNIEnv *env, jobject obj);
jint NativeAdcContinuous_ReadByteArray(FNIEnv *env, jobject obj, jbyteArray b, int off, int count);
jint NativeAdcContinuous_ReadShortArray(FNIEnv *env, jobject obj, jshortArray b, int off, int count);
jint NativeAdcContinuous_ReadIntArray(FNIEnv *env, jobject obj, jintArray b, int off, int count);
jvoid NativeAdcContinuous_Stop(FNIEnv *env, jobject obj);

static constexpr NativeMethod adcContinuousMethods[] = {
    NATIVE_METHOD("start",     "()V",     NativeAdcContinuous_Start),
    NATIVE_METHOD("isStarted", "()Z",     NativeAdcContinuous_IsStart),
    NATIVE_METHOD("read",      "([BII)I", NativeAdcContinuous_ReadByteArray),
    NATIVE_METHOD("read",      "([SII)I", NativeAdcContinuous_ReadShortArray),
    NATIVE_METHOD("read",      "([III)I", NativeAdcContinuous_ReadIntArray),
    NATIVE_METHOD("stop",      "()V",     NativeAdcContinuous_Stop),
};

#endif /* __ESP_NATIVE_ADC_CONTINUOUS_H */
