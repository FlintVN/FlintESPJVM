
#include <stdio.h>
#include <stdlib.h>
#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include "usb_cdc.h"

void USB_CDC_Init(void) {
    // const tinyusb_config_t tusb_cfg = {
    //     .device_descriptor = NULL,
    //     .string_descriptor = NULL,
    //     .external_phy = false,
    //     .configuration_descriptor = NULL,
    // };

    // ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    tinyusb_config_cdcacm_t acm_cfg = { 0 };
    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));
}
