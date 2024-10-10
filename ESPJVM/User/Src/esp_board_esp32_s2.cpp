
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "tusb_cdc_acm.h"
#include "tusb_console.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_usb_device.h"
#include "esp_board.h"

#define BLINK_GPIO              15
#define BUTTON_GPIO             0

static USB_Mode NVS_GetUSBMode(void) {
    nvs_handle_t handle;
    int32_t usbMode = 0;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &handle));
    nvs_get_i32(handle, "usbMode", &usbMode);
    nvs_close(handle);
    return usbMode ? USB_CDC_MSC : USB_CDC;
}

static void NVS_SetUSBMode(USB_Mode usbMode) {
    nvs_handle_t handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &handle));
    ESP_ERROR_CHECK(nvs_set_i32(handle, "usbMode", (int32_t)usbMode));
    nvs_close(handle);
}

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpioNum = (uint32_t)arg;
    if(gpioNum == BUTTON_GPIO) {
        USB_DeviceDisconnect();
        NVS_SetUSBMode((NVS_GetUSBMode() == USB_CDC) ? USB_CDC_MSC : USB_CDC);
        esp_restart();
    }
}

static void GPIO_Init(void) {
    gpio_config_t io_conf = {};

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1 << BLINK_GPIO;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = 1 << BUTTON_GPIO;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);

    gpio_isr_handler_add((gpio_num_t)BUTTON_GPIO, gpio_isr_handler, (void*)BUTTON_GPIO);
}

static void LED_Flash(void) {
    gpio_set_level((gpio_num_t)BLINK_GPIO, 1);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    gpio_set_level((gpio_num_t)BLINK_GPIO, 0);
}

static void NVS_Init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

void Board_Init(void) {
    GPIO_Init();
    LED_Flash();
    NVS_Init();
    vTaskDelay(250 / portTICK_PERIOD_MS);
    USB_Mode usbMode = NVS_GetUSBMode();
    USB_DeviceInit(usbMode);
    esp_tusb_init_console(TINYUSB_CDC_ACM_0);
    if(usbMode == USB_CDC_MSC)
        vTaskDelete(NULL);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}
