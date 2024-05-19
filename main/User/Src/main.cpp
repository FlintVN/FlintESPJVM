
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "tusb_cdc_acm.h"
#include "tusb_console.h"

#include "mjvm.h"
#include "mjvm_system_api.h"
#include "usb_device.h"

#include "tusb_msc_storage.h"

#define BLINK_GPIO              15

static void GPIO_Init() {
    gpio_config_t io_conf = {};

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1 << BLINK_GPIO;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

extern "C" void app_main() {
    GPIO_Init();
    gpio_set_level((gpio_num_t)BLINK_GPIO, 1);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    gpio_set_level((gpio_num_t)BLINK_GPIO, 0);

    USB_DeviceInit(USB_CDC);

    esp_tusb_init_console(TINYUSB_CDC_ACM_0);

    vTaskDelay(2000 / portTICK_PERIOD_MS);

    try {
        const esp_vfs_fat_mount_config_t mount_config = {
            .format_if_mount_failed = true,
            .max_files = 4,
            .allocation_unit_size = CONFIG_WL_SECTOR_SIZE,
            .disk_status_check_enable = false
        };
        static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;
        esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl("", "storage", &mount_config, &s_wl_handle);

        Execution &execution = Mjvm::newExecution();
        int64_t retVal = execution.run("test");
        Mjvm::destroy(execution);
    }
    catch(MjvmThrowable *ex) {
        MjvmString &str = ex->getDetailMessage();
        MjvmSystem_Write(str.getText(), str.getLength(), str.getCoder());
    }
    catch(OutOfMemoryError *err) {
        const char *msg = err->getMessage();
        MjvmSystem_Write(msg, strlen(msg), 0);
    }
    catch(LoadFileError* file) {
        const char *fileName = file->getFileName();
        MjvmSystem_Write("Could not find or load class ", 29, 0);
        MjvmSystem_Write(fileName, strlen(fileName), 0);
    }
    catch(const char *msg) {
        MjvmSystem_Write(msg, strlen(msg), 0);
    }
    MjvmSystem_Write("\n", 1, 0);
}
