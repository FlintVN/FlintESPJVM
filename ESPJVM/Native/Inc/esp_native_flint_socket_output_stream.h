

#ifndef __ESP_NATIVE_FLINT_SOCKET_OUTPUT_STREAM_H
#define __ESP_NATIVE_FLINT_SOCKET_OUTPUT_STREAM_H

#include "flint_native.h"

jvoid NativeFlintSocketOutputStream_SocketWrite(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len);

inline constexpr NativeMethod  flintSocketOutputStreamMethods[] = {
    NATIVE_METHOD("socketWrite", "([BII)V", NativeFlintSocketOutputStream_SocketWrite),
};

#endif /* __ESP_NATIVE_FLINT_SOCKET_OUTPUT_STREAM_H */
