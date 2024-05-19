
#include <stdio.h>
#include "mjvm_system_api.h"

void *MjvmSystem_FileOpen(const char *fileName, MjvmSys_FileMode mode) {
    char buff[6];
    uint32_t index = 0;
    if(mode & (MJVM_FILE_CREATE_ALWAYS | MJVM_FILE_CREATE_NEW)) {
        buff[index++] = 'w';
        if(mode & MJVM_FILE_CREATE_NEW)
            buff[index++] = 'x';
    }
    else
        buff[index++] = 'r';
    buff[index++] = 'b';
    if((mode & (MJVM_FILE_READ | MJVM_FILE_WRITE)) == (MJVM_FILE_READ | MJVM_FILE_WRITE))
        buff[index++] = '+';
    buff[index] = 0;

    return fopen(fileName, buff);
}

MjvmSys_FileResult MjvmSystem_FileRead(void *fileHandle, void *buff, uint32_t btr, uint32_t *br) {
    uint32_t temp = fread(buff, 1, btr, (FILE *)fileHandle);
    *br = temp;
    if(temp != btr)
        return FILE_RESULT_ERR;
    return FILE_RESULT_OK;
}

MjvmSys_FileResult MjvmSystem_FileWrite(void *fileHandle, void *buff, uint32_t btw, uint32_t *bw) {
    uint32_t temp = fwrite(buff , 1, btw, (FILE *)fileHandle);
    *bw = temp;
    if(temp != btw)
        return FILE_RESULT_ERR;
    return FILE_RESULT_OK;
}

uint32_t MjvmSystem_FileSize(void *fileHandle) {
    uint32_t temp = ftell((FILE *)fileHandle);
    if(fseek((FILE *)fileHandle, 0, SEEK_END) != 0)
        throw "error while getting file size";
    uint32_t length = ftell((FILE *)fileHandle);
    if(fseek((FILE *)fileHandle, temp, SEEK_SET) != 0)
        throw "error while getting file size";
    return length;
}

uint32_t MjvmSystem_FileTell(void *fileHandle) {
    return ftell((FILE *)fileHandle);
}

MjvmSys_FileResult MjvmSystem_FileSeek(void *fileHandle, uint32_t offset) {
    return fseek((FILE *)fileHandle, 0, SEEK_SET) != 0 ? FILE_RESULT_ERR : FILE_RESULT_OK;
}

MjvmSys_FileResult MjvmSystem_FileClose(void *fileHandle) {
    if(fileHandle != 0)
        if(fclose((FILE *)fileHandle) != 0)
            return FILE_RESULT_ERR;
    return FILE_RESULT_OK;
}
