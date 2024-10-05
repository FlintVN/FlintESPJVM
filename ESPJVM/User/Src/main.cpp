
#include <stdio.h>
#include "esp_vfs_fat.h"
#include "flint.h"
#include "esp_board.h"
#include "esp_debugger.h"

extern "C" void app_main() {
    static const esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 16,
        .allocation_unit_size = CONFIG_WL_SECTOR_SIZE,
        .disk_status_check_enable = false,
        .use_one_fat = false
    };
    static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

    Board_Init();

    ESP_ERROR_CHECK(esp_vfs_fat_spiflash_mount_rw_wl("", "storage", &mount_config, &s_wl_handle));

    Flint &flint = Flint::getInstance();
    EspDebugger &dbg = EspDebugger::getInstance(flint);
    if(FlintAPI::IO::finfo("main.class", NULL, NULL) == FILE_RESULT_OK)
        flint.runToMain("main");
    vTaskPrioritySet(NULL, 2);
    dbg.receiveTask();
}
