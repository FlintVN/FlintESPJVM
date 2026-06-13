
#ifndef __ESP_DEBUGGER_H
#define __ESP_DEBUGGER_H

#include "flint.h"
#include "flint_system_api.h"

class EspDbg : public FDbg {
private:
    static EspDbg *espDbgInstance;
    EspDbg(void);
    EspDbg(const EspDbg &) = delete;
    void operator=(const EspDbg &) = delete;

    ~EspDbg(void);
public:
    static EspDbg *getInstance(void);
    bool sendData(uint8_t *data, uint32_t length);
    void receiveTask(void);
};

#endif /* __ESP_DEBUGGER_H */
