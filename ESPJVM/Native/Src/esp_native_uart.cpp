
#include <string.h>
#include "flint.h"
#include "soc/uart_pins.h"
#include "driver/uart.h"
#include "flint_system_api.h"
#include "esp_native_common.h"
#include "esp_native_pin.h"
#include "esp_native_uart.h"

typedef class : public JObject {
public:
    jstring getPortName() { return (jstring)getFieldByIndex(0)->getObj(); }
    int32_t getPortId() { return getFieldByIndex(1)->getInt32(); }
    int32_t getBaudrate() { return getFieldByIndex(2)->getInt32(); }
    int32_t getStopBits() { return getFieldByIndex(3)->getInt32(); }
    int32_t getParity() { return getFieldByIndex(4)->getInt32(); }
    int32_t getDataBits() { return getFieldByIndex(5)->getInt32(); }
    int32_t getTxPin() { return getFieldByIndex(6)->getInt32(); }
    int32_t getRxPin() { return getFieldByIndex(7)->getInt32(); }

    void setPortId(int32_t val) { getFieldByIndex(1)->setInt32(val); }
    void setBaudrate(int32_t val) { getFieldByIndex(2)->setInt32(val); }
    void setTxPin(int32_t val) { getFieldByIndex(6)->setInt32(val); }
    void setRxPin(int32_t val) { getFieldByIndex(7)->setInt32(val); }
} *SerialPortObject;

static jobject uartHolder[UART_NUM_MAX] = {};

static bool NativeUart_IsOpen(int32_t uartId) {
    if(0 <= uartId || uartId < LENGTH(uartHolder))
        return uartHolder[uartId] != NULL ? true : false;
    return false;
}

static void NativeUart_Close(int32_t uartId) {
    uart_driver_delete((uart_port_t)uartId);
    uartHolder[uartId] = NULL;
}

void NativeUart_Reset(void) {
#if CONFIG_IDF_TARGET_ESP32
    for(uint8_t i = 1; i < LENGTH(uartHolder); i++)
        NativeUart_Close(i);
#else
    for(uint8_t i = 0; i < LENGTH(uartHolder); i++)
        NativeUart_Close(i);
#endif
}

static int32_t getUartId(FNIEnv *env, jstring portName) {
    if(portName == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "UART name cannot be null");
        return -1;
    }
#if CONFIG_IDF_TARGET_ESP32
    else if(portName->compareTo("UART0", strlen("UART0")) == 0) {
        env->throwNew(env->findClass("java/io/IOException"), "UART0 was used for debugger");
        return -1;
    }
#else
    else if(portName->compareTo("UART0", strlen("UART0")) == 0)
        return UART_NUM_0;
#endif
    else if(portName->compareTo("UART1", strlen("UART1")) == 0)
        return UART_NUM_1;
#if SOC_UART_HP_NUM > 2
    else if(portName->compareTo("UART2", strlen("UART2")) == 0)
        return UART_NUM_2;
#endif
#if SOC_UART_HP_NUM > 3
    else if(portName->compareTo("UART3", strlen("UART3")) == 0)
        return UART_NUM_3;
#endif
#if SOC_UART_HP_NUM > 4
    else if(portName->compareTo("UART4", strlen("UART4")) == 0)
        return UART_NUM_4;
#endif
#if (SOC_UART_LP_NUM >= 1)
    else if(portName->compareTo("LPUART0", strlen("LPUART0")) == 0)
        return LP_UART_NUM_0;
#endif
    else {
        env->throwNew(env->findClass("java/io/IOException"), "Invalid UART name");
        return -1;
    }
}

static void initDefaultValues(SerialPortObject uartObj, int32_t uartId) {
    typedef struct {
        int8_t txPin;
        int8_t rxPin;
    } UartDefaultPin;

    static const UartDefaultPin defaultPins[] = {
        {U0TXD_GPIO_NUM, U0RXD_GPIO_NUM},
        {U1TXD_GPIO_NUM, U1RXD_GPIO_NUM},
#if SOC_UART_HP_NUM > 2
        {U2TXD_GPIO_NUM, U2RXD_GPIO_NUM},
#endif
#if SOC_UART_HP_NUM > 3
        {U3TXD_GPIO_NUM, U3RXD_GPIO_NUM},
#endif
#if SOC_UART_HP_NUM > 4
        {U4TXD_GPIO_NUM, U4RXD_GPIO_NUM},
#endif
#if (SOC_UART_LP_NUM >= 1)
        {LP_U0TXD_GPIO_NUM, LP_U0RXD_GPIO_NUM}
#endif
    };

    uartObj->setPortId(uartId);
    if(uartObj->getTxPin() == -2) uartObj->setTxPin(defaultPins[uartId].txPin);
    if(uartObj->getRxPin() == -2) uartObj->setRxPin(defaultPins[uartId].rxPin);
    if(uartObj->getBaudrate() < 0) uartObj->setBaudrate(9600);
}

static bool checkUartPin(FNIEnv *env, SerialPortObject portObj) {
    const char *msg;
    int32_t txPin = portObj->getTxPin();
    int32_t rxPin = portObj->getRxPin();
    if(txPin == -1 && rxPin == -1) {
        env->throwNew(env->findClass("java/io/IOException"), "TX PIN or RX PIN must be set");
        return false;
    }
    if(txPin >= 0 && (msg = NativePin_CheckPin(txPin)) != NULL) {
        env->throwNew(env->findClass("java/io/IOException"), msg);
        return false;
    }
    if(rxPin >= 0 && (msg = NativePin_CheckPin(rxPin)) != NULL) {
        env->throwNew(env->findClass("java/io/IOException"), msg);
        return false;
    }
    return true;
}

static bool checkPrecondition(FNIEnv *env, SerialPortObject portObj) {
    int32_t portId = portObj->getPortId();
    if(!NativeUart_IsOpen(portId)) {
        env->throwNew(env->findClass("java/io/IOException"), "UART has not been opened yet");
        return false;
    }
    else if(uartHolder[portId] != portObj) {
        env->throwNew(env->findClass("java/io/IOException"), "Access is denied");
        return false;
    }
    return true;
}

jobject nativeUartOpen(FNIEnv *env, jobject obj) {
    SerialPortObject portObj = (SerialPortObject)obj;
    int32_t uartId = getUartId(env, portObj->getPortName());
    if(uartId == -1) return obj;

    initDefaultValues(portObj, uartId);

    if(NativeUart_IsOpen(uartId)) {
        const char *msg = (uartHolder[uartId] != portObj) ? "Access is denied" : "UART is already open";
        env->throwNew(env->findClass("java/io/IOException"), msg);
        return obj;
    }

    if(!checkUartPin(env, portObj)) return obj;

    int32_t parity = portObj->getParity();

    uart_config_t uartConfig = {};
    uartConfig.baud_rate = portObj->getBaudrate();
    uartConfig.data_bits = (uart_word_length_t)(portObj->getDataBits() - 5);
    uartConfig.parity = parity == 0 ? UART_PARITY_DISABLE : (parity == 1 ? UART_PARITY_EVEN : UART_PARITY_ODD);
    uartConfig.stop_bits = (uart_stop_bits_t)(portObj->getStopBits() + 1);
    uartConfig.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    esp_err_t err = uart_param_config((uart_port_t)uartId, &uartConfig);
    if(err == ESP_OK) err = uart_set_pin((uart_port_t)uartId, portObj->getTxPin(), portObj->getRxPin(), UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if(err == ESP_OK) err = uart_driver_install((uart_port_t)uartId, 512 * 2, 0, 0, NULL, 0);
    if(err == ESP_OK)
        uartHolder[uartId] = portObj;
    else
        env->throwNew(env->findClass("java/io/IOException"), "Error while opening UART port");
    return obj;
}

jbool nativeUartIsOpen(FNIEnv *env, jobject obj) {
    SerialPortObject portObj = (SerialPortObject)obj;
    int32_t uartId = portObj->getPortId();
    if(NativeUart_IsOpen(uartId))
        return (uartHolder[uartId] == portObj) ? true : false;
    return false;
}

jint nativeUartGetBaudrate(FNIEnv *env, jobject obj) {
    SerialPortObject portObj = (SerialPortObject)obj;
    int32_t uartId = portObj->getPortId();
    if(NativeUart_IsOpen(uartId)) {
        uint32_t ret;
        if(uart_get_baudrate((uart_port_t)uartId, &ret) == ESP_OK)
            return ret;
    }
    return portObj->getBaudrate();
}

jint nativeUartReadByte(FNIEnv *env, jobject obj) {
    uint8_t buff;
    SerialPortObject portObj = (SerialPortObject)obj;
    int32_t portId = portObj->getPortId();
    if(!checkPrecondition(env, portObj)) return -1;
    while(!env->exec->hasTerminateRequest()) {
        if(uart_read_bytes((uart_port_t)portId, &buff, 1, pdMS_TO_TICKS(10)) == 1)
            return buff;
    }
    return -1;
}

jint nativeUartRead(FNIEnv *env, jobject obj, jbyteArray b, int off, int count) {
    SerialPortObject portObj = (SerialPortObject)obj;
    int32_t portId = portObj->getPortId();
    if(!checkPrecondition(env, portObj)) return 0;
    if(!CheckArrayIndexSize(env, b, off, count)) return 0;
    while(!env->exec->hasTerminateRequest()) {
        int32_t rxSize = uart_read_bytes((uart_port_t)portId, &b->getData()[off], count, pdMS_TO_TICKS(10));
        if(rxSize != 0) return rxSize;
    }
    return 0;
}

jvoid nativeUartWriteByte(FNIEnv *env, jobject obj, int b) {
    uint8_t buff = (uint8_t)b;
    SerialPortObject portObj = (SerialPortObject)obj;
    if(!checkPrecondition(env, portObj)) return;
    if(uart_write_bytes((uart_port_t)portObj->getPortId(), &buff, 1) != 1)
        env->throwNew(env->findClass("java/io/IOException"), "Error while writing data");
}

jvoid nativeUartWrite(FNIEnv *env, jobject obj, jbyteArray b, int off, int count) {
    SerialPortObject portObj = (SerialPortObject)obj;
    if(!checkPrecondition(env, portObj)) return;
    if(!CheckArrayIndexSize(env, b, off, count)) return;
    int8_t *buff = &b->getData()[off];
    int32_t portId = portObj->getPortId();
    while(count) {
        size_t byteSent = uart_write_bytes((uart_port_t)portId, buff, count);
        if(byteSent == 0) {
            env->throwNew(env->findClass("java/io/IOException"), "Error while writing data");
            return;
        }
        count -= byteSent;
        buff += byteSent;
    }
}

jvoid nativeUartClose(FNIEnv *env, jobject obj) {
    SerialPortObject portObj = (SerialPortObject)obj;
    int32_t uartId = portObj->getPortId();
    if(NativeUart_IsOpen(uartId)) {
        if(uartHolder[uartId] != portObj) {
            env->throwNew(env->findClass("java/io/IOException"), "Access is denied");
            return;
        }
        NativeUart_Close(uartId);
    }
    portObj->setPortId(-1);
}
