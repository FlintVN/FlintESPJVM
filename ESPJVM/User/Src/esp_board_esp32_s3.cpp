
#include "driver/gpio.h"
#include "driver/usb_serial_jtag.h"
#include "flint_conf.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_wifi.h"
#include "esp_board.h"

static void GPIO_Init(void) {

}

static void NVS_Init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

static void USBSerialJtag_Init() {
    usb_serial_jtag_driver_config_t usb_serial_jtag_config = {};
    usb_serial_jtag_config.rx_buffer_size = 1024 + 1;
    usb_serial_jtag_config.tx_buffer_size = DBG_TX_BUFFER_SIZE;
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_jtag_config));
}

static void WiFi_Init(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
}

void Board_Init(void) {
    GPIO_Init();
    NVS_Init();
    USBSerialJtag_Init();
    WiFi_Init();
}
