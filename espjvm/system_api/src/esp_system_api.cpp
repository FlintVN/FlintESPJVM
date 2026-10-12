
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

void FlintAPI::System::consoleWrite(uint8_t *utf8, uint32_t length) {
    fprintf(stdout, "%.*s", (int)length, (char *)utf8);
}

int64_t FlintAPI::System::getTimeNanos(void) {
    return esp_timer_get_time() * 1000;
}

int64_t FlintAPI::System::getTimeMillis(void) {
    return esp_timer_get_time() / 1000;
}

bool FlintAPI::System::openClassFile(FExec *ctx, const char *clsName, uint16_t length, FileReader *reader) {
    static const char *jars[] = {
        "/lib/java.base.jar",
        "/lib/flint.io.jar",
        "/lib/flint.net.jar",
        "/lib/flint.drawing.jar",
    };

    if (length == 0xFFFF) length = strlen(clsName);
    for (uint32_t i = 0; i < LENGTH(jars); i++) {
        ZipFileReader zip(ctx, jars[i]);
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
