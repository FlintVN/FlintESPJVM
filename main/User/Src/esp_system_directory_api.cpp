
#include "esp_vfs_fat.h"
#include "flint_system_api.h"

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
