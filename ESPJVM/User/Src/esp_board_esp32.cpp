
#include "esp_board.h"
#include "driver/gpio.h"
#include "driver/uart.h"

#define TXD_PIN             1
#define RXD_PIN             3

static void GPIO_Init(void) {

}

static void UART_Init(uart_port_t uartNum, int32_t baudRate) {
    uart_config_t uart_config = {
        .baud_rate = baudRate,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(uartNum, &uart_config);
    uart_set_pin(uartNum, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void Board_Init(void ) {
    GPIO_Init();
    UART_Init(UART_NUM_0, 921600);
}
