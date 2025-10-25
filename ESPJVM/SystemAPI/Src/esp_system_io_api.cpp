
#include <stdio.h>
#include "flint.h"
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
            fileInfo->size = (fileInfo->directory) ? 0 : fno.fsize;
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
    FIL *fp = (FIL *)Flint::malloc(NULL, sizeof(FIL));
    if(fp == NULL) return NULL;
    if(f_open(fp, fileName, (BYTE)mode) != FR_OK) return NULL;
    return fp;
}

FileResult FlintAPI::IO::fread(FileHandle handle, void *buff, uint32_t btr, uint32_t *br) {
    UINT tmp;
    FileResult ret = convertFileResult(f_read((FIL *)handle, buff, btr, &tmp));
    *br = tmp;
    return ret;
}

FileResult FlintAPI::IO::fwrite(FileHandle handle, void *buff, uint32_t btw, uint32_t *bw) {
    UINT tmp;
    FileResult ret = convertFileResult(f_write((FIL *)handle, buff, btw, &tmp));
    *bw = tmp;
    return ret;
}

uint32_t FlintAPI::IO::fsize(FileHandle handle) {
    return f_size((FIL *)handle);
}

uint32_t FlintAPI::IO::ftell(FileHandle handle) {
    return f_tell((FIL *)handle);
}

FileResult FlintAPI::IO::fseek(FileHandle handle, uint32_t offset) {
    return convertFileResult(f_lseek((FIL *)handle, offset));
}

FileResult FlintAPI::IO::fclose(FileHandle handle) {
    if(handle != NULL) {
        FileResult ret = convertFileResult(f_close((FIL *)handle));
        Flint::free(handle);
        return ret;
    }
    return FILE_RESULT_OK;
}

FileResult FlintAPI::IO::fremove(const char *fileName) {
    return convertFileResult(f_unlink(fileName));
}

FileResult FlintAPI::IO::frename(const char *oldName, const char *newName) {
    return convertFileResult(f_rename(oldName, newName));
}

DirHandle FlintAPI::IO::opendir(const char *dirName) {
    FF_DIR *dir = (FF_DIR *)Flint::malloc(NULL, sizeof(FF_DIR));
    FRESULT ret = f_opendir(dir, dirName);
    if(ret == FR_OK)
        return (void *)dir;
    else {
        Flint::free(dir);
        return NULL;
    }
}

FileResult FlintAPI::IO::readdir(DirHandle handle, FileInfo *fileInfo) {
    FILINFO fno;
    FRESULT ret = f_readdir((FF_DIR *)handle, &fno);
    if(ret == FR_OK) {
        uint16_t index = 0;
        uint16_t year = (fno.fdate >> 9) + 1980;
        uint8_t month = (fno.fdate >> 5) & 0x0F;
        uint8_t day = fno.fdate & 0x1F;
        uint8_t hour = fno.ftime >> 11;
        uint8_t minute = (fno.ftime >> 5) & 0x3F;
        uint8_t second = (fno.ftime & 0x1F) * 2;

        fileInfo->attribute = fno.fattrib;
        fileInfo->size = (fileInfo->directory) ? 0 : fno.fsize;
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
    Flint::free(handle);
    return convertFileResult(ret);
}

FileResult FlintAPI::IO::mkdir(const char *path) {
    return convertFileResult(f_mkdir(path));
}
