
#include <stdio.h>
#include "esp_vfs_fat.h"
#include "flint_system_api.h"

FlintFileResult FlintAPI::File::exists(const char *fileName) {
    FRESULT ret = f_stat(fileName, NULL);
    if(ret == FR_OK)
        return FILE_RESULT_OK;
    else if((ret == FR_NO_PATH) || (ret == FR_NO_FILE) || (ret == FR_INVALID_NAME))
        return FILE_RESULT_NO_PATH;
    else if(ret == FR_DENIED)
        return FILE_RESULT_DENIED;
    else if(ret == FR_WRITE_PROTECTED)
        return FILE_RESULT_WRITE_PROTECTED;
    else
        return FILE_RESULT_ERR;
}

void *FlintAPI::File::open(const char *fileName, FlintFileMode mode) {
    char buff[6];
    uint32_t index = 0;
    if(mode & (FLINT_FILE_CREATE_ALWAYS | FLINT_FILE_CREATE_NEW)) {
        buff[index++] = 'w';
        if(mode & FLINT_FILE_CREATE_NEW)
            buff[index++] = 'x';
    }
    else
        buff[index++] = 'r';
    buff[index++] = 'b';
    if((mode & (FLINT_FILE_READ | FLINT_FILE_WRITE)) == (FLINT_FILE_READ | FLINT_FILE_WRITE))
        buff[index++] = '+';
    buff[index] = 0;

    return fopen(fileName, buff);
}

FlintFileResult FlintAPI::File::read(void *fileHandle, void *buff, uint32_t btr, uint32_t *br) {
    uint32_t temp = fread(buff, 1, btr, (FILE *)fileHandle);
    *br = temp;
    if(temp != btr)
        return FILE_RESULT_ERR;
    return FILE_RESULT_OK;
}

FlintFileResult FlintAPI::File::write(void *fileHandle, void *buff, uint32_t btw, uint32_t *bw) {
    uint32_t temp = fwrite(buff , 1, btw, (FILE *)fileHandle);
    *bw = temp;
    if(temp != btw)
        return FILE_RESULT_ERR;
    return FILE_RESULT_OK;
}

uint32_t FlintAPI::File::size(void *fileHandle) {
    uint32_t temp = ftell((FILE *)fileHandle);
    if(fseek((FILE *)fileHandle, 0, SEEK_END) != 0)
        throw "error while getting file size";
    uint32_t length = ftell((FILE *)fileHandle);
    if(fseek((FILE *)fileHandle, temp, SEEK_SET) != 0)
        throw "error while getting file size";
    return length;
}

uint32_t FlintAPI::File::tell(void *fileHandle) {
    return ftell((FILE *)fileHandle);
}

FlintFileResult FlintAPI::File::seek(void *fileHandle, uint32_t offset) {
    return fseek((FILE *)fileHandle, offset, SEEK_SET) != 0 ? FILE_RESULT_ERR : FILE_RESULT_OK;
}

FlintFileResult FlintAPI::File::close(void *fileHandle) {
    if(fileHandle != 0)
        if(fclose((FILE *)fileHandle) != 0)
            return FILE_RESULT_ERR;
    return FILE_RESULT_OK;
}

FlintFileResult FlintAPI::File::remove(const char *fileName) {
    FRESULT ret = f_unlink(fileName);
    if(ret == FR_OK)
        return FILE_RESULT_OK;
    else if((ret == FR_NO_PATH) || (ret == FR_NO_FILE) || (ret == FR_INVALID_NAME))
        return FILE_RESULT_NO_PATH;
    else if(ret == FR_DENIED)
        return FILE_RESULT_DENIED;
    else if(ret == FR_WRITE_PROTECTED)
        return FILE_RESULT_WRITE_PROTECTED;
    else
        return FILE_RESULT_ERR;
}
