
#ifndef __ESP_NATIVE_COMMON_H
#define __ESP_NATIVE_COMMON_H

#include "flint_std.h"
#include "flint_native_interface.h"

bool CheckArrayIndexSize(FNIEnv *env, jarray arr, int32_t index, int32_t count);

#endif /* __ESP_NATIVE_COMMON_H */
