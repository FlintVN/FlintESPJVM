
#include <stdio.h>
#include "flint.h"
#include "esp_system.h"
#include "esp_vfs_fat.h"
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

    Flint::setCwd("/");
    Flint::setClassPaths("/lib/java.base;/lib/flint.io;/lib/flint.net");

    if(esp_reset_reason() != ESP_RST_PANIC) {
        if(FlintAPI::IO::finfo("Main.class", NULL) == FlintAPI::IO::FILE_RESULT_OK)
            Flint::runToMain("Main");
    }

    vTaskPrioritySet(NULL, 2);
    EspDbg::getInstance()->receiveTask();
}
