
#include <stdio.h>
#include "esp_vfs_fat.h"
#include "flint_common.h"
#include "flint_system_api.h"

FlintFileResult FlintAPI::IO::finfo(const char *fileName, uint32_t *size, int64_t *time) {
    FRESULT ret;
    if(size || time) {
        FILINFO fno;
        ret = ::f_stat(fileName, &fno);
        if(ret == FR_OK) {
            if(size)
                *size = fno.fsize;
            if(time) {
                uint16_t year = (fno.fdate >> 9) + 1980;
                uint8_t month = (fno.fdate >> 5) & 0x0F;
                uint8_t day = fno.fdate & 0x1F;
                uint8_t hour = fno.ftime >> 11;
                uint8_t minute = (fno.ftime >> 5) & 0x3F;
                uint8_t second = (fno.ftime & 0x1F) * 2;
                *time = Flint_GetUnixTime(year, month, day, hour, minute, second);
            }
        }
    }
    else
        ret = ::f_stat(fileName, NULL);
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

void *FlintAPI::IO::fopen(const char *fileName, FlintFileMode mode) {
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

    return ::fopen(fileName, buff);
}

FlintFileResult FlintAPI::IO::fread(void *handle, void *buff, uint32_t btr, uint32_t *br) {
    uint32_t temp = ::fread(buff, 1, btr, (FILE *)handle);
    *br = temp;
    if(temp != btr)
        return FILE_RESULT_ERR;
    return FILE_RESULT_OK;
}

FlintFileResult FlintAPI::IO::fwrite(void *handle, void *buff, uint32_t btw, uint32_t *bw) {
    uint32_t temp = ::fwrite(buff , 1, btw, (FILE *)handle);
    *bw = temp;
    if(temp != btw)
        return FILE_RESULT_ERR;
    return FILE_RESULT_OK;
}

uint32_t FlintAPI::IO::fsize(void *handle) {
    uint32_t temp = ::ftell((FILE *)handle);
    if(::fseek((FILE *)handle, 0, SEEK_END) != 0)
        throw "error while getting file size";
    uint32_t length = ::ftell((FILE *)handle);
    if(::fseek((FILE *)handle, temp, SEEK_SET) != 0)
        throw "error while getting file size";
    return length;
}

uint32_t FlintAPI::IO::ftell(void *handle) {
    return ::ftell((FILE *)handle);
}

FlintFileResult FlintAPI::IO::fseek(void *handle, uint32_t offset) {
    return ::fseek((FILE *)handle, offset, SEEK_SET) != 0 ? FILE_RESULT_ERR : FILE_RESULT_OK;
}

FlintFileResult FlintAPI::IO::fclose(void *handle) {
    if(handle != 0)
        if(::fclose((FILE *)handle) != 0)
            return FILE_RESULT_ERR;
    return FILE_RESULT_OK;
}

FlintFileResult FlintAPI::IO::fremove(const char *fileName) {
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

void *FlintAPI::IO::opendir(const char *dirName) {
    FF_DIR *dir = (FF_DIR *)FlintAPI::System::malloc(sizeof(FF_DIR));
    FRESULT ret = f_opendir(dir, dirName);
    if(ret == FR_OK)
        return (void *)dir;
    else {
        FlintAPI::System::free(dir);
        return 0;
    }
}

FlintFileResult FlintAPI::IO::readdir(void *handle, uint8_t *attribute, char *nameBuff, uint32_t buffSize, uint32_t *size, int64_t *time) {
    FILINFO fno;
    FRESULT ret = f_readdir((FF_DIR *)handle, &fno);
    if(ret == FR_OK && fno.fname[0]) {
        uint16_t index = 0;
        uint16_t year = (fno.fdate >> 9) + 1980;
        uint8_t month = (fno.fdate >> 5) & 0x0F;
        uint8_t day = fno.fdate & 0x1F;
        uint8_t hour = fno.ftime >> 11;
        uint8_t minute = (fno.ftime >> 5) & 0x3F;
        uint8_t second = (fno.ftime & 0x1F) * 2;
        *attribute = fno.fattrib;
        *size = fno.fsize;
        *time = Flint_GetUnixTime(year, month, day, hour, minute, second);
        while(fno.fname[index]) {
            if(index < buffSize) {
                nameBuff[index] = fno.fname[index];
                index++;
            }
            else
                return FILE_RESULT_ERR;
        }
        nameBuff[index] = 0;
        return FILE_RESULT_OK;
    }
    else if((ret == FR_NO_PATH) || (ret == FR_NO_FILE) || (ret == FR_INVALID_NAME))
        return FILE_RESULT_NO_PATH;
    else if(ret == FR_DENIED)
        return FILE_RESULT_DENIED;
    else
        return FILE_RESULT_ERR;
}

FlintFileResult FlintAPI::IO::closedir(void *handle) {
    FRESULT ret = f_closedir((FF_DIR *)handle);
    FlintAPI::System::free(handle);
    return (ret == FR_OK) ? FILE_RESULT_OK : FILE_RESULT_ERR;
}

FlintFileResult FlintAPI::IO::mkdir(const char *path) {
    FRESULT ret = f_mkdir(path);
    if(ret == FR_OK)
        return FILE_RESULT_OK;
    else if(ret == FR_DENIED)
        return FILE_RESULT_DENIED;
    else if(ret == FR_WRITE_PROTECTED)
        return FILE_RESULT_WRITE_PROTECTED;
    else
        return FILE_RESULT_ERR;
}
