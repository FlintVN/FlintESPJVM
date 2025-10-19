
#include <stdio.h>
#include "esp_vfs_fat.h"
#include "flint_common.h"
#include "flint_system_api.h"

using namespace FlintAPI::IO;

static FileResult convertFileResult(FRESULT ret) {
    static const FileResult map[] = {
        [FR_OK] = FILE_RESULT_OK,
        [FR_DISK_ERR] = FILE_RESULT_ERR,
        [FR_INT_ERR] = FILE_RESULT_ERR,
        [FR_NOT_READY] = FILE_RESULT_ERR,
        [FR_NO_FILE] = FILE_RESULT_NO_PATH,
        [FR_NO_PATH] = FILE_RESULT_NO_PATH,
        [FR_INVALID_NAME] = FILE_RESULT_NO_PATH,
        [FR_DENIED] = FILE_RESULT_DENIED,
        [FR_EXIST] = FILE_RESULT_ERR,
        [FR_INVALID_OBJECT] = FILE_RESULT_ERR,
        [FR_WRITE_PROTECTED] = FILE_RESULT_WRITE_PROTECTED,
        [FR_INVALID_DRIVE] = FILE_RESULT_ERR,
        [FR_NOT_ENABLED] = FILE_RESULT_ERR,
        [FR_NO_FILESYSTEM] = FILE_RESULT_ERR,
        [FR_MKFS_ABORTED] = FILE_RESULT_ERR,
        [FR_TIMEOUT] = FILE_RESULT_ERR,
        [FR_LOCKED]= FILE_RESULT_ERR,
        [FR_NOT_ENOUGH_CORE] = FILE_RESULT_ERR,
        [FR_TOO_MANY_OPEN_FILES] = FILE_RESULT_ERR,
        [FR_INVALID_PARAMETER] = FILE_RESULT_ERR,
    };
    return map[ret];
}

FileResult FlintAPI::IO::finfo(const char *fileName, FileInfo *fileInfo) {
    FRESULT ret;
    if(fileInfo != NULL) {
        FILINFO fno;
        ret = f_stat(fileName, &fno);
        if(ret == FR_OK) {
            uint16_t index = 0;
            uint16_t year = (fno.fdate >> 9) + 1980;
            uint8_t month = (fno.fdate >> 5) & 0x0F;
            uint8_t day = fno.fdate & 0x1F;
            uint8_t hour = fno.ftime >> 11;
            uint8_t minute = (fno.ftime >> 5) & 0x3F;
            uint8_t second = (fno.ftime & 0x1F) * 2;

            fileInfo->attribute = fno.fattrib;
            fileInfo->size = fno.fsize;
            fileInfo->time = UnixTime(year, month, day, hour, minute, second);
            while(fno.fname[index] != 0) {
                if(index < (sizeof(fileInfo->name) - 1)) {
                    fileInfo->name[index] = fno.fname[index];
                    index++;
                }
                else
                    return FILE_RESULT_ERR;
            }
            fileInfo->name[index] = 0;
        }
    }
    else
        ret = f_stat(fileName, NULL);
    return convertFileResult(ret);
}

FileHandle FlintAPI::IO::fopen(const char *fileName, FileMode mode) {
    char buff[6];
    uint32_t index = 0;
    if(mode & (FILE_MODE_CREATE_ALWAYS | FILE_MODE_CREATE_NEW)) {
        buff[index++] = 'w';
        if(mode & FILE_MODE_CREATE_NEW)
            buff[index++] = 'x';
    }
    else
        buff[index++] = 'r';
    buff[index++] = 'b';
    if((mode & (FILE_MODE_READ | FILE_MODE_WRITE)) == (FILE_MODE_READ | FILE_MODE_WRITE))
        buff[index++] = '+';
    buff[index] = 0;

    return ::fopen(fileName, buff);
}

FileResult FlintAPI::IO::fread(FileHandle handle, void *buff, uint32_t btr, uint32_t *br) {
    uint32_t temp = ::fread(buff, 1, btr, (FILE *)handle);
    *br = temp;
    if(temp != btr)
        return FILE_RESULT_ERR;
    return FILE_RESULT_OK;
}

FileResult FlintAPI::IO::fwrite(FileHandle handle, void *buff, uint32_t btw, uint32_t *bw) {
    uint32_t temp = ::fwrite(buff , 1, btw, (FILE *)handle);
    *bw = temp;
    if(temp != btw)
        return FILE_RESULT_ERR;
    return FILE_RESULT_OK;
}

uint32_t FlintAPI::IO::ftell(FileHandle handle) {
    return ::ftell((FILE *)handle);
}

FileResult FlintAPI::IO::fseek(FileHandle handle, uint32_t offset) {
    return ::fseek((FILE *)handle, offset, SEEK_SET) != 0 ? FILE_RESULT_ERR : FILE_RESULT_OK;
}

FileResult FlintAPI::IO::fclose(FileHandle handle) {
    if(handle != 0)
        if(::fclose((FILE *)handle) != 0)
            return FILE_RESULT_ERR;
    return FILE_RESULT_OK;
}

FileResult FlintAPI::IO::fremove(const char *fileName) {
    return convertFileResult(f_unlink(fileName));
}

FileResult FlintAPI::IO::frename(const char *oldName, const char *newName) {
    return convertFileResult(f_rename(oldName, newName));
}

DirHandle FlintAPI::IO::opendir(const char *dirName) {
    FF_DIR *dir = (FF_DIR *)FlintAPI::System::malloc(sizeof(FF_DIR));
    FRESULT ret = f_opendir(dir, dirName);
    if(ret == FR_OK)
        return (void *)dir;
    else {
        FlintAPI::System::free(dir);
        return NULL;
    }
}

FileResult FlintAPI::IO::readdir(DirHandle handle, FileInfo *fileInfo) {
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

        fileInfo->attribute = fno.fattrib;
        fileInfo->size = fno.fsize;
        fileInfo->time = UnixTime(year, month, day, hour, minute, second);
        while(fno.fname[index] != 0) {
            if(index < (sizeof(fileInfo->name) - 1)) {
                fileInfo->name[index] = fno.fname[index];
                index++;
            }
            else
                return FILE_RESULT_ERR;
        }
        fileInfo->name[index] = 0;
    }
    return convertFileResult(ret);
}

FileResult FlintAPI::IO::closedir(DirHandle handle) {
    FRESULT ret = f_closedir((FF_DIR *)handle);
    FlintAPI::System::free(handle);
    return convertFileResult(ret);
}

FileResult FlintAPI::IO::mkdir(const char *path) {
    return convertFileResult(f_mkdir(path));
}
