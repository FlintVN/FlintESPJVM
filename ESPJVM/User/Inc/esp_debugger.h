
#ifndef __ESP_DEBUGGER_H
#define __ESP_DEBUGGER_H

#include "flint.h"
#include "flint_system_api.h"

class EspDebugger : public FlintDebugger {
private:
    static EspDebugger *espDbgInstance;
    EspDebugger(Flint &flint);
    EspDebugger(const EspDebugger &) = delete;
    void operator=(const EspDebugger &) = delete;

    ~EspDebugger(void);
public:
    static EspDebugger &getInstance(Flint &flint);
    bool sendData(uint8_t *data, uint32_t length);
    void receiveTask(void);
};

#endif /* __ESP_DEBUGGER_H */
