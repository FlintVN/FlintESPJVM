
#include "esp_vfs_fat.h"
#include "flint_system_api.h"

void *FlintAPI::Directory::open(const char *dirName) {
    FF_DIR *dir = (FF_DIR *)FlintAPI::System::malloc(sizeof(FF_DIR));
    FRESULT ret = f_opendir(dir, dirName);
    if(ret == FR_OK)
        return (void *)dir;
    else {
        FlintAPI::System::free(dir);
        return 0;
    }
}

FlintFileResult FlintAPI::Directory::read(void *handle, bool *isFile, char *nameBuff, uint32_t buffSize) {
    FILINFO fno;
    FRESULT ret = f_readdir((FF_DIR *)handle, &fno);
    if(ret == FR_OK && fno.fname[0]) {
        uint16_t index = 0;
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

FlintFileResult FlintAPI::Directory::close(void *handle) {
    FRESULT ret = f_closedir((FF_DIR *)handle);
    FlintAPI::System::free(handle);
    return (ret == FR_OK) ? FILE_RESULT_OK : FILE_RESULT_ERR;
}

FlintFileResult FlintAPI::Directory::exists(const char *path) {
    FRESULT ret = f_stat(path, NULL);
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

FlintFileResult FlintAPI::Directory::create(const char *path) {
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

FlintFileResult FlintAPI::Directory::remove(const char *path) {
    FRESULT ret = f_unlink(path);
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
