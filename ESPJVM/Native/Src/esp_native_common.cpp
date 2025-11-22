
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "flint_array_object.h"
#include "esp_native_common.h"

static portMUX_TYPE atomicMux = portMUX_INITIALIZER_UNLOCKED;

bool CheckArrayIndexSize(FNIEnv *env, jarray arr, int32_t index, int32_t count) {
    if(arr == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return false;
    }
    else if(index < 0) {
        jclass excpCls = env->findClass("java/lang/ArrayIndexOutOfBoundsException");
        uint16_t len;
        const char *name = arr->getCompTypeName(&len);
        env->throwNew(excpCls, "index %d out of bounds for %.*s[%d]", index, len, name, arr->getLength());
        return false;
    }
    else if((index + count) > arr->getLength()) {
        jclass excpCls = env->findClass("java/lang/ArrayIndexOutOfBoundsException");
        uint16_t len;
        const char *name = arr->getCompTypeName(&len);
        env->throwNew(excpCls, "last index %d out of bounds for %.*s[%d]", index + count - 1, len, name, arr->getLength());
        return false;
    }
    return true;
}

void EnterAtomicSection(void) {
    portENTER_CRITICAL(&atomicMux);
}

void ExitAtomicSection(void) {
    portEXIT_CRITICAL(&atomicMux);
}
