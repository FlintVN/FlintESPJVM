
#include <errno.h>
#include <dirent.h>
#include "esp_check.h"
#include "esp_partition.h"
#include "driver/gpio.h"
#include "tinyusb.h"
#include "tinyusb_cdc_acm.h"
#include "tinyusb_default_config.h"
#include "esp_usb_device.h"

#define CDC_INTERFACE_IDX       0x00

#define OUT_ENPOINT(id)         (0x7F & (id))
#define IN_ENPOINT(id)          (0x80 | (id))

static const uint8_t cdc_desc_configuration[] = {
    /* Configuration Descriptor */
    0x09,                               /* bLength */
    TUSB_DESC_CONFIGURATION,            /* bDescriptorType */
    U16_TO_U8S_LE(0x004B),              /* wTotalLength */
    0x02,                               /* bNumInterfaces */
    0x01,                               /* bConfigurationValue */
    0x00,                               /* iConfiguration */
    0xE0,                               /* bmAttributes -  Self Powered */
    100 >> 1,                           /* MaxPower - 100mA */

    /* IAD Descriptor */
    0x08,                               /* bLength */
    TUSB_DESC_INTERFACE_ASSOCIATION,    /* bDescriptorType */
    CDC_INTERFACE_IDX,                  /* bFirstInterface */
    0x02,                               /* bInterfaceCount */
    0x02,                               /* bFunctionClass - CDC Control */
    0x02,                               /* bFunctionSubClass */
    0x00,                               /* bFunctionProtocol */
    0x00,                               /* iFunction */

    /* (CDC Control) USB Device Interface Class Descriptor */
    0x09,                               /* bLength */
    TUSB_DESC_INTERFACE,                /* bDescriptorType */
    CDC_INTERFACE_IDX,                  /* bInterfaceNumber */
    0x00,                               /* bAlternateSetting */
    0x01,                               /* bNumEndpoints */
    0x02,                               /* bInterfaceClass */
    0x02,                               /* bInterfaceSubClass */
    0x00,                               /* bInterfaceProtocol */
    0x00,                               /* iInterface */

    /* Header Functional Descriptor */
    0x05,                               /* bLength */
    0x24,                               /* bDescriptorType */
    0x00,                               /* bDescriptorSubtype - Header Func Desc */
    0x10,                               /* bcdCDC - Spec release number */
    0x01,

    /* Call Management Functional Descriptor */
    0x05,                               /* bLength */
    0x24,                               /* bDescriptorType */
    0x01,                               /* bDescriptorSubtype - Call Management Functional Descriptor */
    0x00,                               /* bmCapabilities - D0 + D1 */
    CDC_INTERFACE_IDX + 1,              /* bDataInterface */

    /* ACM Functional Descriptor */
    0x04,                               /* bLength */
    0x24,                               /* bDescriptorType */
    0x02,                               /* bDescriptorSubtype - Abstract Control Management Descriptor */
    0x02,                               /* bmCapabilities */

    /* Union Functional Descriptor */
    0x05,                               /* bLength */
    0x24,                               /* bDescriptorType */
    0x06,                               /* bDescriptorSubtype - Union Functional Descriptor */
    CDC_INTERFACE_IDX,                  /* bMasterInterface - Communication class interface */
    CDC_INTERFACE_IDX + 1,              /* bSlaveInterface0 - Data Class Interface */

    /* IN Endpoint 2 Descriptor */
    0x07,                               /* bLength */
    TUSB_DESC_ENDPOINT,                 /* bDescriptorType */
    IN_ENPOINT(0x02),                   /* bEndpointAddress */
    0x03,                               /* bmAttributes - Interrupt Transfer Type */
    U16_TO_U8S_LE(0x0008),              /* wMaxPacketSize - 8 bytes */
    0x10,                               /* bInterval */

    /* CDC Data USB Device Interface Class Descriptor */
    0x09,                               /* bLength */
    TUSB_DESC_INTERFACE,                /* bDescriptorType */
    CDC_INTERFACE_IDX + 1,              /* bInterfaceNumber */
    0x00,                               /* bAlternateSetting */
    0x02,                               /* bNumEndpoints */
    0x0A,                               /* bInterfaceClass */
    0x00,                               /* bInterfaceSubClass */
    0x00,                               /* bInterfaceProtocol */
    0x00,                               /* iInterface */

    /* OUT Endpoint 3 Descriptor */
    0x07,                               /* bLength */
    TUSB_DESC_ENDPOINT,                 /* bDescriptorType */
    OUT_ENPOINT(0x03),                  /* bEndpointAddress */
    0x02,                               /* bmAttributes -  Bulk Transfer Type */
    U16_TO_U8S_LE(0x0040),              /* wMaxPacketSize - 64 bytes */
    0x00,                               /* bInterval */

    /* IN Endpoint 3 Descriptor */
    0x07,                               /* bLength */
    TUSB_DESC_ENDPOINT,                 /* bDescriptorType */
    IN_ENPOINT(0x03),                   /* bEndpointAddress */
    0x02,                               /* bmAttributes - Bulk Transfer Type */
    U16_TO_U8S_LE(0x0040),              /* wMaxPacketSize - 64 bytes */
    0x00,                               /* bInterval */
};

static tusb_desc_device_t descriptor_config = {
    .bLength = sizeof(descriptor_config),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x303A,
    .idProduct = 0x0002,
    .bcdDevice = 0x0723,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01
};

static const char *string_desc_arr[] = {
    (const char[]){0x09, 0x04},     /* 0: is supported language is English (0x0409) */
    "Espressif",                    /* 1: Manufacturer */
    "ESP32-S2",                     /* 2: Product */
    "0",                            /* 3: Serials */
};

void USB_DeviceInit(void) {
    tinyusb_config_cdcacm_t acmCfg = {
        .cdc_port = TINYUSB_CDC_ACM_0,
        .callback_rx = NULL,
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = NULL,
        .callback_line_coding_changed = NULL
    };

    ESP_ERROR_CHECK(tinyusb_cdcacm_init(&acmCfg));

    tinyusb_config_t tusb_cfg = TINYUSB_DEFAULT_CONFIG();
    tusb_cfg.descriptor.device = &descriptor_config;
    tusb_cfg.descriptor.full_speed_config = cdc_desc_configuration;
    tusb_cfg.descriptor.string = string_desc_arr;
    tusb_cfg.descriptor.string_count = sizeof(string_desc_arr) / sizeof(string_desc_arr[0]);
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
}
