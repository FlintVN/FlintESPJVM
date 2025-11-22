

#ifndef __ESP_NATIVE_UART_H
#define __ESP_NATIVE_UART_H

#include "flint.h"
#include "flint_native.h"

void NativeUart_Reset(void);

jobject nativeUartOpen(FNIEnv *env, jobject obj);
jbool nativeUartIsOpen(FNIEnv *env, jobject obj);
jint nativeUartGetBaudrate(FNIEnv *env, jobject obj);
jint nativeUartReadByte(FNIEnv *env, jobject obj);
jint nativeUartRead(FNIEnv *env, jobject obj, jbyteArray b, int off, int count);
jvoid nativeUartWriteByte(FNIEnv *env, jobject obj, int b);
jvoid nativeUartWrite(FNIEnv *env, jobject obj, jbyteArray b, int off, int count);
jvoid nativeUartClose(FNIEnv *env, jobject obj);

static constexpr NativeMethod uartMethods[] = {
    NATIVE_METHOD("open",        "()Lflint/machine/SerialPort;", nativeUartOpen),
    NATIVE_METHOD("isOpen",      "()Z",                          nativeUartIsOpen),
    NATIVE_METHOD("getBaudrate", "()I",                          nativeUartGetBaudrate),
    NATIVE_METHOD("read",        "()I",                          nativeUartReadByte),
    NATIVE_METHOD("read",        "([BII)I",                      nativeUartRead),
    NATIVE_METHOD("write",       "(I)V",                         nativeUartWriteByte),
    NATIVE_METHOD("write",       "([BII)V",                      nativeUartWrite),
    NATIVE_METHOD("close",       "()V",                          nativeUartClose),
};

#endif /* __ESP_NATIVE_UART_H */
