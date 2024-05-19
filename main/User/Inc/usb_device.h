
#ifndef __USB_DEVICE_H
#define __USB_DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    USB_CDC = 0,
    USB_CDC_MSC,
} USB_Mode;

void USB_DeviceDisconnect(void);
void USB_DeviceInit(USB_Mode mode);

#ifdef __cplusplus
}
#endif

#endif /* __USB_DEVICE_H */
