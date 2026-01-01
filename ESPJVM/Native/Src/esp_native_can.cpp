#include <string.h>
#include "sdkconfig.h"

extern "C" {
#include "driver/twai.h"
}

#include "flint.h"
#include "flint_java_object.h"
#include "flint_system_api.h"
#include "esp_native_can.h"

/* =========================================================
 * Helpers
 * ========================================================= */

static twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_NC, GPIO_NUM_NC, TWAI_MODE_NO_ACK);
static twai_timing_config_t g_timing = {};
static twai_filter_config_t g_filter = TWAI_FILTER_CONFIG_ACCEPT_ALL();

static bool g_is_initialized = false;
static bool g_is_started = false;

static void set_native_handle(FNIEnv *env, jobject obj, void* handle) {
    FieldValue *field = obj->getField(env->exec, "nativeHandle");
    if (field) {
        field->setInt64(reinterpret_cast<int64_t>(handle));
    }
}

/* =========================================================
 * Native API
 * ========================================================= */

void NativeCan_Reset(void) {
    if (g_is_started) {
        twai_stop();
        g_is_started = false;
    }
    if (g_is_initialized) {
        twai_driver_uninstall();
        g_is_initialized = false;
    }
}

/* --------------------------------------------------------- */

jobject NativeCan_Open(FNIEnv *env, jobject obj) {
    FieldValue *txField = obj->getField(env->exec, "txPin");
    FieldValue *rxField = obj->getField(env->exec, "rxPin");
    FieldValue *baudField = obj->getField(env->exec, "baudRate");
    FieldValue *sampleField = obj->getField(env->exec, "samplePoint");

    int txPin = txField ? txField->getInt32() : -1;
    int rxPin = rxField ? rxField->getInt32() : -1;
    int bitrate = baudField ? baudField->getInt32() : 0;
    int samplePoint = sampleField ? sampleField->getInt32() : 0;

    if (txPin < 0 || rxPin < 0) {
        env->throwNew(env->findClass("java/io/IOException"), "TX/RX pin not set");
        return nullptr;
    }

    if (bitrate <= 0) {
        env->throwNew(env->findClass("java/io/IOException"), "Invalid bitrate");
        return nullptr;
    }

    g_config.tx_io = (gpio_num_t)txPin;
    g_config.rx_io = (gpio_num_t)rxPin;

    if (samplePoint <= 0) {
        switch (bitrate) {
            case 1000000: g_timing = TWAI_TIMING_CONFIG_1MBITS(); break;
            case 800000:  g_timing = TWAI_TIMING_CONFIG_800KBITS(); break;
            case 500000:  g_timing = TWAI_TIMING_CONFIG_500KBITS(); break;
            case 250000:  g_timing = TWAI_TIMING_CONFIG_250KBITS(); break;
            case 125000:  g_timing = TWAI_TIMING_CONFIG_125KBITS(); break;
            case 100000:  g_timing = TWAI_TIMING_CONFIG_100KBITS(); break;
            case 50000:   g_timing = TWAI_TIMING_CONFIG_50KBITS(); break;
            case 25000:   g_timing = TWAI_TIMING_CONFIG_25KBITS(); break;
            default:
                uint32_t clock_hz = 80000000;
                uint32_t brp = clock_hz / (bitrate * 16);
                brp = (brp < 2) ? 2 : ((brp > 128) ? 128 : brp);
                g_timing.brp = brp;
                g_timing.tseg_1 = 13;
                g_timing.tseg_2 = 2;
                g_timing.sjw = 1;
                g_timing.triple_sampling = false;
                break;
        }
    } else {
        uint32_t clock_hz = 80000000;
        uint32_t total_quanta = 16;
        uint32_t brp = clock_hz / (bitrate * total_quanta);
        brp = (brp < 2) ? 2 : ((brp > 128) ? 128 : brp);
        g_timing.brp = brp;
        g_timing.tseg_1 = (total_quanta * samplePoint / 1000) - 1;
        g_timing.tseg_2 = total_quanta - 1 - g_timing.tseg_1;
        g_timing.sjw = 1;
        g_timing.triple_sampling = false;
    }

    esp_err_t err = twai_driver_install(&g_config, &g_timing, &g_filter);
    if (err != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), "Failed to install TWAI driver");
        return nullptr;
    }

    g_is_initialized = true;
    set_native_handle(env, obj, (void*)1); // Dummy handle cho tương thích
    return obj;
}

/* --------------------------------------------------------- */

jvoid NativeCan_Close(FNIEnv *env, jobject obj) {
    NativeCan_Stop(env, obj);
    
    if (g_is_initialized) {
        esp_err_t err = twai_driver_uninstall();
        if (err != ESP_OK) {
            env->throwNew(env->findClass("java/io/IOException"), "Failed to uninstall TWAI driver");
            return;
        }
        g_is_initialized = false;
    }
    
    set_native_handle(env, obj, nullptr);
}

/* --------------------------------------------------------- */

jbool NativeCan_IsOpen(FNIEnv *env, jobject obj) {
    return g_is_initialized;
}

/* --------------------------------------------------------- */

jvoid NativeCan_Start(FNIEnv *env, jobject obj) {
    if (!g_is_initialized) {
        env->throwNew(env->findClass("java/io/IOException"), "CAN not initialized");
        return;
    }

    esp_err_t err = twai_start();
    if (err != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), "Failed to start TWAI");
        return;
    }
    
    g_is_started = true;
}

/* --------------------------------------------------------- */

jvoid NativeCan_Stop(FNIEnv *env, jobject obj) {
    if (g_is_started) {
        esp_err_t err = twai_stop();
        if (err != ESP_OK) {
            env->throwNew(env->findClass("java/io/IOException"), "Failed to stop TWAI");
            return;
        }
        g_is_started = false;
    }
}

/* --------------------------------------------------------- */

jvoid NativeCan_Recover(FNIEnv *env, jobject obj) {
    esp_err_t err = twai_initiate_recovery();
    if (err != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), "Failed to initiate TWAI recovery");
    }
}

/* --------------------------------------------------------- */

jvoid NativeCan_ConfigureAlerts(FNIEnv *env, jobject obj, jlong alertsMask) {
    esp_err_t err = twai_reconfigure_alerts((uint32_t)alertsMask, NULL);
    if (err != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), "Failed to configure TWAI alerts");
    }
}

/* --------------------------------------------------------- */

jlong NativeCan_ReadAlerts(FNIEnv *env, jobject obj, jlong timeoutMs) {
    uint32_t alerts;
    esp_err_t err = twai_read_alerts(&alerts, pdMS_TO_TICKS(timeoutMs));
    if (err != ESP_OK && err != ESP_ERR_TIMEOUT) {
        env->throwNew(env->findClass("java/io/IOException"), "Failed to read TWAI alerts");
        return 0;
    }
    return (jlong)alerts;
}

/* --------------------------------------------------------- */

jobject NativeCan_GetStatusInfo(FNIEnv *env, jobject obj) {
    if (!g_is_initialized) {
        return nullptr;
    }

    twai_status_info_t st;
    esp_err_t err = twai_get_status_info(&st);
    if (err != ESP_OK) {
        return nullptr;
    }

    JClass *cls = env->findClass("flint/machine/Can$StatusInfo");
    jobject info = env->newObject(cls);
    if (!info) return nullptr;

    info->getField(env->exec, "txErrorCounter")->setInt32(st.tx_error_counter);
    info->getField(env->exec, "rxErrorCounter")->setInt32(st.rx_error_counter);
    info->getField(env->exec, "busOff")->setBool(st.state == TWAI_STATE_BUS_OFF);

    return info;
}

/* --------------------------------------------------------- */

jvoid NativeCan_ClearTxQueue(FNIEnv *env, jobject obj) {
    if (g_is_started) {
        twai_stop();
        twai_start();
    }
}

/* --------------------------------------------------------- */

jvoid NativeCan_ClearRxQueue(FNIEnv *env, jobject obj) {
    twai_message_t frame;
    while (twai_receive(&frame, 0) == ESP_OK) {
        // Discard
    }
}

/* --------------------------------------------------------- */

jvoid NativeCan_Write(FNIEnv *env, jobject obj, jbyteArray data) {
    if (!g_is_started) {
        env->throwNew(env->findClass("java/io/IOException"), "CAN not started");
        return;
    }

    int len = 0;
    if (data) {
        FieldValue *lenField = data->getField(env->exec, "length");
        if (lenField) len = lenField->getInt32();
    }
    if (len > 8) len = 8;

    twai_message_t frame = {};
    frame.data_length_code = len;
    frame.identifier = 0;
    frame.flags = TWAI_MSG_FLAG_NONE;

    uint8_t buffer[8] = {0};
    // TODO: Copy dữ liệu thực từ Java byte[] vào buffer
    // Hiện tại gửi toàn 0 để compile thành công và test cơ bản

    memcpy(frame.data, buffer, len);

    esp_err_t err = twai_transmit(&frame, pdMS_TO_TICKS(1000));
    if (err != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), "CAN transmit failed");
    }
}

/* --------------------------------------------------------- */

jbyteArray NativeCan_ReadFrame(FNIEnv *env, jobject obj, jlong timeoutMs) {
    if (!g_is_started) {
        return nullptr;
    }

    twai_message_t frame;
    esp_err_t err = twai_receive(&frame, pdMS_TO_TICKS(timeoutMs));
    if (err != ESP_OK) {
        return nullptr;
    }

    jbyteArray arr = env->newByteArray(frame.data_length_code);
    if (arr && frame.data_length_code > 0) {
        // TODO: Copy dữ liệu thực từ frame.data vào Java byte[]
    }
    return arr;
}

/* --------------------------------------------------------- */

jobject NativeCan_ReadMessage(FNIEnv *env, jobject obj, jlong timeoutMs) {
    return NativeCan_ReceiveMessage(env, obj, timeoutMs);
}

/* --------------------------------------------------------- */

jvoid NativeCan_WriteMessage(FNIEnv *env, jobject obj, jobject msg, jlong timeoutMs) {
    if (!g_is_started) {
        env->throwNew(env->findClass("java/io/IOException"), "CAN not started");
        return;
    }

    FieldValue *idField = msg->getField(env->exec, "id");
    FieldValue *dataField = msg->getField(env->exec, "data");
    int id = idField ? idField->getInt32() : 0;
    jbyteArray data = dataField ? (jbyteArray)dataField->getObj() : nullptr;

    int len = 0;
    if (data) {
        FieldValue *lenField = data->getField(env->exec, "length");
        if (lenField) len = lenField->getInt32();
    }
    if (len > 8) len = 8;

    twai_message_t frame = {};
    frame.identifier = id;
    frame.data_length_code = len;
    frame.flags = (id > 0x7FF) ? TWAI_MSG_FLAG_EXTD : TWAI_MSG_FLAG_NONE;

    uint8_t buffer[8] = {0};
    // TODO: Copy dữ liệu thực từ Java byte[]
    memcpy(frame.data, buffer, len);

    esp_err_t err = twai_transmit(&frame, pdMS_TO_TICKS(timeoutMs));
    if (err != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), "CAN send failed");
    }
}

/* --------------------------------------------------------- */

jobject NativeCan_ReceiveMessage(FNIEnv *env, jobject obj, jlong timeoutMs) {
    if (!g_is_started) {
        return nullptr;
    }

    twai_message_t frame;
    esp_err_t err = twai_receive(&frame, pdMS_TO_TICKS(timeoutMs));
    if (err != ESP_OK) {
        return nullptr;
    }

    JClass *cls = env->findClass("flint/machine/Can$CanMessage");
    jobject msg = env->newObject(cls);
    if (!msg) return nullptr;

    msg->getField(env->exec, "id")->setInt32(frame.identifier);

    jbyteArray arr = env->newByteArray(frame.data_length_code);
    if (arr && frame.data_length_code > 0) {
        // TODO: Copy dữ liệu thực vào arr
    }

    msg->getField(env->exec, "data")->setObj(arr);
    return msg;
}