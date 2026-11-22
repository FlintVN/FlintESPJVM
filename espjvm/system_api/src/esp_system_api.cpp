
#include <stdio.h>
#include "sdkconfig.h"
#include "esp_timer.h"
#include "flint_utf8.h"
#include "esp_heap_caps.h"
#include "../heap_private.h"
#include "flint_system_api.h"
#include "flint_file_reader.h"
#include "flint_zip_file_reader.h"

void *FlintAPI::System::malloc(uint32_t size) {
    return ::malloc(size);
}

void *FlintAPI::System::realloc(void *p, uint32_t size) {
    return ::realloc(p, size);
}

void FlintAPI::System::free(void *p) {
    ::free(p);
}

uint32_t FlintAPI::System::freeMemory(void) {
    return heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
}

uint32_t FlintAPI::System::totalMemory(void) {
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_SPIRAM);
    return info.total_free_bytes + info.total_allocated_bytes;
}

void FlintAPI::System::consoleWrite(uint8_t *utf8, uint32_t length) {
    fprintf(stdout, "%.*s", (int)length, (char *)utf8);
}

int64_t FlintAPI::System::getTimeNanos(void) {
    return esp_timer_get_time() * 1000;
}

int64_t FlintAPI::System::getTimeMillis(void) {
    return esp_timer_get_time() / 1000;
}

const char *FlintAPI::System::getClassPath(uint32_t index) {
    static const char *jars[] = {
        "/lib/java.base.jar",
        "/lib/flint.io.jar",
        "/lib/flint.net.jar",
        "/lib/flint.drawing.jar",
    };
    if (index < LENGTH(jars))
        return jars[index];
    return NULL;
}

bool FlintAPI::System::openClassFile(FExec *ctx, const char *clsName, uint16_t length, FileReader *reader) {
    if (length == 0xFFFF) length = strlen(clsName);
    for (uint32_t i = 0; true; i++) {
        const char *jar = FlintAPI::System::getClassPath(i);
        if (jar == NULL) break;
        ZipFileReader zip(ctx, jar);
        if (zip.open()) {
            if (zip.openClassFileEntry(clsName, length)) {
                memcpy(reader, &zip, sizeof(FileReader));
                return true;
            }
            zip.close();
        }
    }
    return false;
}
