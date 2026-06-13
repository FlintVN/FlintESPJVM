

#ifndef __ESP_NATIVE_TOUCH_PAD_H
#define __ESP_NATIVE_TOUCH_PAD_H

#include "flint_native.h"

void NativeTouchPad_Reset(void);

jint NativeTouchPad_InitTouchPad(FNIEnv *env, jobject obj, jint pin);
jint NativeTouchPad_Read(FNIEnv *env, jobject obj);
jbool NativeTouchPad_IsTouched(FNIEnv *env, jobject obj);

inline constexpr NativeMethod touchPadMethods[] = {
    NATIVE_METHOD("initTouchPad", "(I)I", NativeTouchPad_InitTouchPad),
    NATIVE_METHOD("read",         "()I",  NativeTouchPad_Read),
    NATIVE_METHOD("isTouched",    "()Z",  NativeTouchPad_IsTouched),
};

#endif /* __ESP_NATIVE_TOUCH_PAD_H */
