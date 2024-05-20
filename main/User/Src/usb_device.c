
#include <errno.h>
#include <dirent.h>
#include "esp_check.h"
#include "esp_partition.h"
#include "driver/gpio.h"
#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include "tusb_msc_storage.h"
#include "usb_device.h"
#include "tusb_tasks.h"

#define EPNUM_MSC               1
#define TUSB_DESC_TOTAL_LEN     (TUD_CONFIG_DESC_LEN + TUD_MSC_DESC_LEN)

#define CDC_INTERFACE_IDX       0x00
#define MSC_INTERFACE_IDX       0x02

#define OUT_ENPOINT(id)         (0x7F & (id))
#define IN_ENPOINT(id)          (0x80 | (id))

static uint8_t const cdc_desc_configuration[] = {
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
    0x04,                               /* iInterface */

    /* Header Functional Descriptor */
    0x05,                               /* bLength */
    0x24,                               /* bDescriptorType */
    0x00,                               /* bDescriptorSubtype - Header Func Desc */
    0x20,                               /* bcdCDC - Spec release number */
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

static uint8_t const cdc_msc_desc_configuration[] = {
    /* Configuration Descriptor */
    0x09,                               /* bLength */
    TUSB_DESC_CONFIGURATION,            /* bDescriptorType */
    U16_TO_U8S_LE(0x0062),              /* wTotalLength */
    0x03,                               /* bNumInterfaces */
    0x01,                               /* bConfigurationValue */
    0x00,                               /* iConfiguration */
    0xA0,                               /* bmAttributes - Bus Powered*/
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
    0x04,                               /* iInterface */

    /* Header Functional Descriptor */
    0x05,                               /* bLength */
    0x24,                               /* bDescriptorType */
    0x00,                               /* bDescriptorSubtype - Header Func Desc */
    0x20,                               /* bcdCDC - Spec release number */
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

    /* Mass Storage USB Device Interface Class Descriptor */
    0x09,                               /* bLength */
    TUSB_DESC_INTERFACE,                /* bDescriptorType */
    MSC_INTERFACE_IDX,                  /* bInterfaceNumber */
    0x00,                               /* bAlternateSetting */
    0x02,                               /* bNumEndpoints */
    0x08,                               /* bInterfaceClass - Mass Storage USB Device Interface Class */
    0x06,                               /* bInterfaceSubClass */
    0x50,                               /* bInterfaceProtocol */
    0x00,                               /* iInterface */

    /* OUT Endpoint 1 Descriptor */
    0x07,                               /* bLength */
    TUSB_DESC_ENDPOINT,                 /* bDescriptorType */
    OUT_ENPOINT(0x01),                  /* bEndpointAddress */
    0x02,                               /* bmAttributes - Bulk Transfer Type*/
    U16_TO_U8S_LE(0x0040),              /* wMaxPacketSize - 64 byte */
    0x00,                               /* bInterval */

    /* IN Endpoint 1 Descriptor */
    0x07,                               /* bLength */
    TUSB_DESC_ENDPOINT,                 /* bDescriptorType */
    IN_ENPOINT(0x01),                   /* bEndpointAddress */
    0x02,                               /* bmAttributes - Bulk Transfer Type */
    U16_TO_U8S_LE(0x0040),              /* wMaxPacketSize - 64 byte */
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
    .idProduct = 0x4002,
    .bcdDevice = 0x100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01
};

static char const *string_desc_arr[] = {
    (const char[]){0x09, 0x04},     /* 0: is supported language is English (0x0409) */
    "TinyUSB",                      /* 1: Manufacturer */
    "TinyUSB Device",               /* 2: Product */
    "123456",                       /* 3: Serials */
    "Example MSC",                  /* 4. MSC */
};

static void storage_mount_changed_cb(tinyusb_msc_event_t *event) {
    /* bool isMounted = event->mount_changed_data.is_mounted; */
}

static esp_err_t storage_init_spiflash(wl_handle_t *wl_handle) {
    const esp_partition_t *data_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, NULL);
    if (data_partition == NULL)
        return ESP_ERR_NOT_FOUND;

    return wl_mount(data_partition, wl_handle);
}

void USB_DeviceDisconnect(void) {
    // tinyusb_msc_storage_unmount();
    // tinyusb_msc_storage_deinit();
    tusb_cdc_acm_deinit(TINYUSB_CDC_ACM_0);
    tinyusb_driver_uninstall();
    tusb_stop_task();
}

void USB_DeviceInit(USB_Mode mode) {
    tinyusb_config_cdcacm_t acmCfg = {
        .usb_dev = TINYUSB_USBDEV_0,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .rx_unread_buf_sz = 64,
        .callback_rx = NULL,
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = NULL,
        .callback_line_coding_changed = NULL
    };
    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acmCfg));
    
    if(mode == USB_CDC_MSC) {
        static wl_handle_t wl_handle = WL_INVALID_HANDLE;
        ESP_ERROR_CHECK(storage_init_spiflash(&wl_handle));
        const tinyusb_msc_spiflash_config_t config_spi = {
            .wl_handle = wl_handle,
            .callback_mount_changed = storage_mount_changed_cb,
            .mount_config.max_files = 5,
        };
        ESP_ERROR_CHECK(tinyusb_msc_storage_init_spiflash(&config_spi));
        ESP_ERROR_CHECK(tinyusb_msc_register_callback(TINYUSB_MSC_EVENT_MOUNT_CHANGED, storage_mount_changed_cb));
        ESP_ERROR_CHECK(tinyusb_msc_storage_mount("/data"));

        const tinyusb_config_t tusb_cfg = {
            .device_descriptor = &descriptor_config,
            .string_descriptor = string_desc_arr,
            .string_descriptor_count = sizeof(string_desc_arr) / sizeof(string_desc_arr[0]),
            .external_phy = false,
            .configuration_descriptor = cdc_msc_desc_configuration,
        };
        ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    }
    else {
        const tinyusb_config_t tusb_cfg = {
            .device_descriptor = &descriptor_config,
            .string_descriptor = string_desc_arr,
            .string_descriptor_count = sizeof(string_desc_arr) / sizeof(string_desc_arr[0]),
            .external_phy = false,
            .configuration_descriptor = cdc_desc_configuration,
        };
        ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    }
}
