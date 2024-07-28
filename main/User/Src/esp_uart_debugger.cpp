
#include <iostream>
#include "tusb_cdc_acm.h"
#include "esp_uart_debugger.h"

static EspUartDebugger *espUartDbg = 0;

static void cdcRxCallback(int itf, cdcacm_event_t *event) {
    static uint8_t buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];
    size_t rx_size = 0;
    esp_err_t ret = tinyusb_cdcacm_read(TINYUSB_CDC_ACM_0, buf, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size);
    if(ret == ESP_OK) {
        if(espUartDbg)
            espUartDbg->receivedDataHandler(buf, rx_size);
    }
}

EspUartDebugger::EspUartDebugger(Flint &flint) : FlintDebugger(flint) {
    ESP_ERROR_CHECK(tinyusb_cdcacm_register_callback(
        TINYUSB_CDC_ACM_0,
        CDC_EVENT_RX,
        cdcRxCallback
    ));
}

EspUartDebugger &EspUartDebugger::getInstance(Flint &flint) {
    if(espUartDbg == 0) {
        espUartDbg = (EspUartDebugger *)Flint::malloc(sizeof(EspUartDebugger));
        new (espUartDbg)EspUartDebugger(flint);
    }
    return *espUartDbg;
}

bool EspUartDebugger::sendData(uint8_t *data, uint32_t length) {
    while(length) {
        size_t queueSize = tinyusb_cdcacm_write_queue(TINYUSB_CDC_ACM_0, data, length);
        if(tinyusb_cdcacm_write_flush(TINYUSB_CDC_ACM_0, 0) != ESP_OK)
            return false;
        length -= queueSize;
        data += queueSize;
    }
    return true;
}

EspUartDebugger::~EspUartDebugger(void) {

}
