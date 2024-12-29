

#ifndef __ESP_SYSTEM_NATIVE_PIN_H
#define __ESP_SYSTEM_NATIVE_PIN_H

#include "flint_native_class.h"

extern const FlintNativeClass PIN_CLASS;

const char *NativePin_CheckPin(int32_t pin);
void NativePin_Reset(Flint &flint);

#endif /* __ESP_SYSTEM_NATIVE_PIN_H */
