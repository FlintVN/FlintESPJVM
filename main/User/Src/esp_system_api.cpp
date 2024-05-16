
#include "mjvm_system_api.h"

void MjvmSystem_WriteChar(uint16_t ch) {
    throw "MjvmSystem_WriteChar is not implemented in VM";
}

int64_t MjvmSystem_GetNanoTime(void) {
    throw "MjvmSystem_GetNanoTime is not implemented in VM";
}

void *MjvmSystem_FileOpen(const char *fileName, MjvmSys_FileMode mode) {
    throw "MjvmSystem_FileOpen is not implemented in VM";
}

MjvmSys_FileResult MjvmSystem_FileRead(void *fileHandle, void *buff, uint32_t btr, uint32_t *br) {
    throw "MjvmSystem_FileRead is not implemented in VM";
}

MjvmSys_FileResult MjvmSystem_FileWrite(void *fileHandle, void *buff, uint32_t btw, uint32_t *bw) {
    throw "MjvmSystem_FileWrite is not implemented in VM";
}

uint32_t MjvmSystem_FileSize(void *fileHandle) {
    throw "MjvmSystem_FileSize is not implemented in VM";
}

uint32_t MjvmSystem_FileTell(void *fileHandle) {
    throw "MjvmSystem_FileTell is not implemented in VM";
}

MjvmSys_FileResult MjvmSystem_FileSeek(void *fileHandle, uint32_t offset) {
    throw "MjvmSystem_FileSeek is not implemented in VM";
}

MjvmSys_FileResult MjvmSystem_FileClose(void *fileHandle) {
    throw "MjvmSystem_FileClose is not implemented in VM";
}
