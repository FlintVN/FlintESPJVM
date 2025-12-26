

#ifndef __ESP_NATIVE_UART_H
#define __ESP_NATIVE_UART_H

#include "flint.h"
#include "flint_native.h"

void NativeUart_Reset(void);

jobject NativeUart_Open(FNIEnv *env, jobject obj);
jbool NativeUart_IsOpen(FNIEnv *env, jobject obj);
jint NativeUart_GetBaudrate(FNIEnv *env, jobject obj);
jint NativeUart_ReadByte(FNIEnv *env, jobject obj);
jint NativeUart_Read(FNIEnv *env, jobject obj, jbyteArray b, int off, int count);
jvoid NativeUart_WriteByte(FNIEnv *env, jobject obj, int b);
jvoid NativeUart_Write(FNIEnv *env, jobject obj, jbyteArray b, int off, int count);
jvoid NativeUart_Close(FNIEnv *env, jobject obj);

inline constexpr NativeMethod uartMethods[] = {
    NATIVE_METHOD("open",        "()Lflint/machine/SerialPort;", NativeUart_Open),
    NATIVE_METHOD("isOpen",      "()Z",                          NativeUart_IsOpen),
    NATIVE_METHOD("getBaudrate", "()I",                          NativeUart_GetBaudrate),
    NATIVE_METHOD("read",        "()I",                          NativeUart_ReadByte),
    NATIVE_METHOD("read",        "([BII)I",                      NativeUart_Read),
    NATIVE_METHOD("write",       "(I)V",                         NativeUart_WriteByte),
    NATIVE_METHOD("write",       "([BII)V",                      NativeUart_Write),
    NATIVE_METHOD("close",       "()V",                          NativeUart_Close),
};

#endif /* __ESP_NATIVE_UART_H */
