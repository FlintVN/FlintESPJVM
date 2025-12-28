

#ifndef __ESP_NATIVE_FLINT_SOCKET_INPUT_STREAM_H
#define __ESP_NATIVE_FLINT_SOCKET_INPUT_STREAM_H

#include "flint_native.h"

jint NativeFlintSocketInputStream_SocketRead(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len);

inline constexpr NativeMethod flintSocketInputStreamMethods[] = {
    NATIVE_METHOD("socketRead", "([BII)I", NativeFlintSocketInputStream_SocketRead),
};

#endif /* __ESP_NATIVE_FLINT_SOCKET_INPUT_STREAM_H */
