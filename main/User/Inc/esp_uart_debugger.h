
#ifndef __ESP_UART_DEBUGGER_H
#define __ESP_UART_DEBUGGER_H

#include "flint.h"
#include "flint_system_api.h"

class EspUartDebugger : public FlintDebugger {
private:
    EspUartDebugger(Flint &flint);
    EspUartDebugger(const EspUartDebugger &) = delete;
    void operator=(const EspUartDebugger &) = delete;

    ~EspUartDebugger(void);
public:
    static EspUartDebugger &getInstance(Flint &flint);
    bool sendData(uint8_t *data, uint32_t length);
    void receiveTask(void);
};

#endif /* __ESP_UART_DEBUGGER_H */
