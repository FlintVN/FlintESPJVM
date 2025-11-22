
#ifndef __ESP_NATIVE_COMMON_H
#define __ESP_NATIVE_COMMON_H

#include "flint_std.h"
#include "flint_native_interface.h"

bool CheckArrayIndexSize(FNIEnv *env, jarray arr, int32_t index, int32_t count);

void EnterAtomicSection(void);
void ExitAtomicSection(void);

__attribute__((always_inline)) static inline uint32_t GetCpuTicks(void) {
    uint32_t ccount;
#if CONFIG_IDF_TARGET_ARCH_RISCV
    __asm__ __volatile__ ("csrr %0, 0x7E2" : "=r" (ccount));
#else
    __asm__ __volatile__ ("rsr %0, ccount" : "=a" (ccount));
#endif
    return (uint32_t)ccount;
}

#endif /* __ESP_NATIVE_COMMON_H */
