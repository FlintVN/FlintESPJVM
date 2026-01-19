#include <string.h>
#include <stdlib.h>
#include "sdkconfig.h"
#include "flint.h"
#include "flint_java_object.h"
#include "flint_system_api.h"
#include "flint_class_loader.h"
#include "esp_native_can.h"

extern "C" {
#include "driver/twai.h"
}

/* =========================================================
 * CAN Context Structure (Multi-instance support)
 * ========================================================= */

typedef struct {
    twai_general_config_t g_config;
    twai_timing_config_t g_timing;
    twai_filter_config_t g_filter;
    bool is_initialized;
    bool is_started;
    int controller_id;
} CANContext;

// Static array for multiple CAN instances (max 2 instances for ESP32)
static CANContext g_can_contexts[2] = {};

/* =========================================================
 * Context Management
 * ========================================================= */

static CANContext* get_context(FNIEnv *env, jobject obj) {
    FieldValue *field = obj->getField(env->exec, "nativeHandle");
    uintptr_t handle = (uintptr_t)field->getInt64();
    return (CANContext*)handle;
}

static void set_context(FNIEnv *env, jobject obj, CANContext* ctx) {
    FieldValue *field = obj->getField(env->exec, "nativeHandle");
    field->setInt64((int64_t)(uintptr_t)ctx);
}

static int get_controller_id(FNIEnv *env, jobject obj) {
    FieldValue *field = obj->getField(env->exec, "controllerId");
    return field ? field->getInt32() : 0;
}

static int get_pin_field(FNIEnv *env, jobject obj, const char* field_name, int default_pin) {
    FieldValue *field = obj->getField(env->exec, field_name);
    return field ? field->getInt32() : default_pin;
}

static uint32_t calculate_timing_from_bitrate(int bitrate, int sample_point) {
    // APB clock frequency for ESP32 (80MHz)
    const uint32_t APB_CLK_HZ = 80000000;
    
    // Default timing if sample point not specified
    if (sample_point <= 0 || sample_point > 1000) {
        sample_point = 750; // Default 75%
    }
    
    // Total time quanta per bit
    const uint32_t total_quanta = 16;
    uint32_t tseg1 = (total_quanta * sample_point) / 1000;
    uint32_t tseg2 = total_quanta - tseg1;
    
    // Calculate BRP
    uint32_t brp = APB_CLK_HZ / (bitrate * total_quanta);
    
    // Clamp values
    if (brp < 2) brp = 2;
    if (brp > 128) brp = 128;
    if (tseg1 < 1) tseg1 = 1;
    if (tseg1 > 16) tseg1 = 16;
    if (tseg2 < 1) tseg2 = 1;
    if (tseg2 > 8) tseg2 = 8;
    
    return (brp << 16) | ((tseg1 - 1) << 8) | ((tseg2 - 1) << 4) | 1;
}

/* =========================================================
 * Common Helper Functions
 * ========================================================= */

static jbyteArray can_frame_to_bytearray(FNIEnv *env, const twai_message_t* frame) {
    jbyteArray arr = env->newByteArray(frame->data_length_code);
    if (arr && frame->data_length_code > 0) {
        memcpy(arr->getData(), frame->data, frame->data_length_code);
    }
    return arr;
}

static void bytearray_to_can_frame(FNIEnv *env, jbyteArray data, twai_message_t* frame) {
    if (!data) return;
    
    jint len = data->getLength();
    if (len > 8) len = 8;
    
    frame->data_length_code = len;
    if (len > 0) {
        memcpy(frame->data, data->getData(), len);
    }
}

/* =========================================================
 * Native API Implementation
 * ========================================================= */

void NativeCan_Reset(void) {
    for (int i = 0; i < 2; i++) {
        CANContext* ctx = &g_can_contexts[i];
        if (ctx->is_started) {
            twai_stop();
            ctx->is_started = false;
        }
        if (ctx->is_initialized) {
            twai_driver_uninstall();
            ctx->is_initialized = false;
        }
    }
}

/* --------------------------------------------------------- */

jobject NativeCan_Open(FNIEnv *env, jobject obj) {
    int controller_id = get_controller_id(env, obj);
    
    // Validate controller ID
    if (controller_id < 0 || controller_id >= 2) {
        env->throwNew(env->findClass("java/io/IOException"), 
                      "Invalid CAN controller ID");
        return nullptr;
    }
    
    // Check if already initialized
    CANContext* ctx = &g_can_contexts[controller_id];
    if (ctx->is_initialized) {
        env->throwNew(env->findClass("java/io/IOException"), 
                      "CAN controller already initialized");
        return nullptr;
    }
    
    // Get configuration from Java object
    int tx_pin = get_pin_field(env, obj, "txPin", -1);
    int rx_pin = get_pin_field(env, obj, "rxPin", -1);
    int bitrate = get_pin_field(env, obj, "baudRate", 0);
    int sample_point = get_pin_field(env, obj, "samplePoint", 750);
    
    // Validate pins
    if (tx_pin < 0 || rx_pin < 0) {
        env->throwNew(env->findClass("java/io/IOException"), 
                      "TX/RX pins not configured");
        return nullptr;
    }
    
    // Calculate timing
    uint32_t timing = calculate_timing_from_bitrate(bitrate, sample_point);
    
    // Configure TWAI
    ctx->g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        (gpio_num_t)tx_pin, 
        (gpio_num_t)rx_pin, 
        TWAI_MODE_NORMAL
    );
    
    // Set timing configuration based on bitrate
    switch (bitrate) {
        case 1000000:
            ctx->g_timing = TWAI_TIMING_CONFIG_1MBITS();
            break;
        case 500000:
            ctx->g_timing = TWAI_TIMING_CONFIG_500KBITS();
            break;
        case 250000:
            ctx->g_timing = TWAI_TIMING_CONFIG_250KBITS();
            break;
        case 125000:
            ctx->g_timing = TWAI_TIMING_CONFIG_125KBITS();
            break;
        case 100000:
            ctx->g_timing = TWAI_TIMING_CONFIG_100KBITS();
            break;
        default: {
            // Custom timing
            ctx->g_timing.brp = (timing >> 16) & 0xFF;
            ctx->g_timing.tseg_1 = ((timing >> 8) & 0xF) + 1;
            ctx->g_timing.tseg_2 = ((timing >> 4) & 0x7) + 1;
            ctx->g_timing.sjw = timing & 0x7;
            ctx->g_timing.triple_sampling = false;
            break;
        }
    }
    
    ctx->g_filter = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    ctx->controller_id = controller_id;
    
    // Install driver
    esp_err_t err = twai_driver_install(&ctx->g_config, &ctx->g_timing, &ctx->g_filter);
    if (err != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), 
                      "Failed to initialize CAN driver");
        return nullptr;
    }
    
    ctx->is_initialized = true;
    set_context(env, obj, ctx);
    return obj;
}

/* --------------------------------------------------------- */

jbool NativeCan_IsOpen(FNIEnv *env, jobject obj) {
    CANContext* ctx = get_context(env, obj);
    return ctx && ctx->is_initialized;
}

/* --------------------------------------------------------- */

jvoid NativeCan_Start(FNIEnv *env, jobject obj) {
    CANContext* ctx = get_context(env, obj);
    if (!ctx || !ctx->is_initialized) {
        env->throwNew(env->findClass("java/io/IOException"), 
                      "CAN not initialized");
        return;
    }
    
    esp_err_t err = twai_start();
    if (err != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), 
                      "Failed to start CAN");
        return;
    }
    
    ctx->is_started = true;
}

/* --------------------------------------------------------- */

jvoid NativeCan_Stop(FNIEnv *env, jobject obj) {
    CANContext* ctx = get_context(env, obj);
    if (!ctx || !ctx->is_started) {
        return;
    }
    
    esp_err_t err = twai_stop();
    if (err != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), 
                      "Failed to stop CAN");
        return;
    }
    
    ctx->is_started = false;
}

/* --------------------------------------------------------- */

jvoid NativeCan_Close(FNIEnv *env, jobject obj) {
    CANContext* ctx = get_context(env, obj);
    if (!ctx) {
        return;
    }
    
    // Stop if running
    if (ctx->is_started) {
        twai_stop();
        ctx->is_started = false;
    }
    
    // Uninstall driver
    if (ctx->is_initialized) {
        esp_err_t err = twai_driver_uninstall();
        if (err != ESP_OK) {
            env->throwNew(env->findClass("java/io/IOException"), 
                          "Failed to uninstall CAN driver");
            return;
        }
        ctx->is_initialized = false;
    }
    
    set_context(env, obj, nullptr);
}

/* --------------------------------------------------------- */

jvoid NativeCan_Recover(FNIEnv *env, jobject obj) {
    esp_err_t err = twai_initiate_recovery();
    if (err != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), 
                      "Failed to initiate CAN recovery");
    }
}

/* --------------------------------------------------------- */

jvoid NativeCan_ConfigureAlerts(FNIEnv *env, jobject obj, jlong alertsMask) {
    esp_err_t err = twai_reconfigure_alerts((uint32_t)alertsMask, NULL);
    if (err != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), 
                      "Failed to configure CAN alerts");
    }
}

/* --------------------------------------------------------- */

jlong NativeCan_ReadAlerts(FNIEnv *env, jobject obj, jlong timeoutMs) {
    uint32_t alerts = 0;
    esp_err_t err = twai_read_alerts(&alerts, pdMS_TO_TICKS(timeoutMs));
    
    if (err != ESP_OK && err != ESP_ERR_TIMEOUT) {
        env->throwNew(env->findClass("java/io/IOException"), 
                      "Failed to read CAN alerts");
        return 0;
    }
    
    return (jlong)alerts;
}

/* --------------------------------------------------------- */

jvoid NativeCan_Write(FNIEnv *env, jobject obj, jbyteArray data) {
    CANContext* ctx = get_context(env, obj);
    if (!ctx || !ctx->is_started) {
        env->throwNew(env->findClass("java/io/IOException"), 
                      "CAN not started");
        return;
    }
    
    twai_message_t frame;
    memset(&frame, 0, sizeof(frame));
    frame.flags = TWAI_MSG_FLAG_NONE;
    
    bytearray_to_can_frame(env, data, &frame);
    
    esp_err_t err = twai_transmit(&frame, pdMS_TO_TICKS(1000));
    if (err != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), 
                      "CAN transmit failed");
    }
}

/* --------------------------------------------------------- */

jbyteArray NativeCan_ReadFrame(FNIEnv *env, jobject obj, jlong timeoutMs) {
    CANContext* ctx = get_context(env, obj);
    if (!ctx || !ctx->is_started) {
        return nullptr;
    }
    
    twai_message_t frame;
    esp_err_t err = twai_receive(&frame, pdMS_TO_TICKS(timeoutMs));
    if (err != ESP_OK) {
        return nullptr;
    }
    
    return can_frame_to_bytearray(env, &frame);
}

/* --------------------------------------------------------- */

jvoid NativeCan_WriteMessage(FNIEnv *env, jobject obj, jobject msg, jlong timeoutMs) {
    CANContext* ctx = get_context(env, obj);
    if (!ctx || !ctx->is_started) {
        env->throwNew(env->findClass("java/io/IOException"), 
                      "CAN not started");
        return;
    }
    
    // Extract message fields
    FieldValue *id_field = msg->getField(env->exec, "id");
    FieldValue *ext_field = msg->getField(env->exec, "extended");
    FieldValue *rtr_field = msg->getField(env->exec, "rtr");
    FieldValue *data_field = msg->getField(env->exec, "data");
    
    if (!id_field) {
        env->throwNew(env->findClass("java/io/IOException"), 
                      "Invalid message: id field missing");
        return;
    }
    
    twai_message_t frame;
    memset(&frame, 0, sizeof(frame));
    
    // Set identifier
    frame.identifier = id_field->getInt32();
    
    // Set flags
    frame.flags = TWAI_MSG_FLAG_NONE;
    if (ext_field && ext_field->getBool()) {
        frame.flags |= TWAI_MSG_FLAG_EXTD;
    }
    if (rtr_field && rtr_field->getBool()) {
        frame.flags |= TWAI_MSG_FLAG_RTR;
    }
    
    // Set data if not RTR frame
    if (!(frame.flags & TWAI_MSG_FLAG_RTR) && data_field) {
        jbyteArray data = (jbyteArray)data_field->getObj();
        bytearray_to_can_frame(env, data, &frame);
    }
    
    // Transmit
    esp_err_t err = twai_transmit(&frame, pdMS_TO_TICKS(timeoutMs));
    if (err != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), 
                      "CAN send failed");
    }
}

/* --------------------------------------------------------- */

jobject NativeCan_ReceiveMessage(FNIEnv *env, jobject obj, jlong timeoutMs) {
    CANContext* ctx = get_context(env, obj);
    if (!ctx || !ctx->is_started) {
        return nullptr;
    }
    
    twai_message_t frame;
    esp_err_t err = twai_receive(&frame, pdMS_TO_TICKS(timeoutMs));
    if (err != ESP_OK) {
        return nullptr;
    }
    
    // Create Java CanMessage object
    JClass *cls = env->findClass("flint/machine/CanMessage");
    if (!cls) {
        return nullptr;
    }
    
    jobject msg = env->newObject(cls);
    if (!msg) {
        return nullptr;
    }
    
    // Set ID
    msg->getField(env->exec, "id")->setInt32(frame.identifier);
    
    // Set extended flag
    bool is_extended = (frame.flags & TWAI_MSG_FLAG_EXTD) != 0;
    msg->getField(env->exec, "extended")->setBool(is_extended);
    
    // Set RTR flag
    bool is_rtr = (frame.flags & TWAI_MSG_FLAG_RTR) != 0;
    msg->getField(env->exec, "rtr")->setBool(is_rtr);
    
    // Set DLC
    msg->getField(env->exec, "dlc")->setInt32(frame.data_length_code);
    
    // Set data if not RTR frame
    if (!is_rtr && frame.data_length_code > 0) {
        jbyteArray arr = can_frame_to_bytearray(env, &frame);
        msg->getField(env->exec, "data")->setObj(arr);
    } else {
        // Empty byte array for RTR frames
        jbyteArray arr = env->newByteArray(0);
        msg->getField(env->exec, "data")->setObj(arr);
    }
    
    return msg;
}

/* --------------------------------------------------------- */

jobject NativeCan_ReadMessage(FNIEnv *env, jobject obj, jlong timeoutMs) {
    return NativeCan_ReceiveMessage(env, obj, timeoutMs);
}

/* --------------------------------------------------------- */

jobject NativeCan_GetStatusInfo(FNIEnv *env, jobject obj) {
    CANContext* ctx = get_context(env, obj);
    if (!ctx || !ctx->is_initialized) {
        return nullptr;
    }
    
    twai_status_info_t st;
    esp_err_t err = twai_get_status_info(&st);
    if (err != ESP_OK) {
        return nullptr;
    }
    
    JClass *cls = env->findClass("flint/machine/CanStatus");
    if (!cls) {
        return nullptr;
    }
    
    jobject info = env->newObject(cls);
    if (!info) {
        return nullptr;
    }
    
    // Set error counters
    info->getField(env->exec, "txErrorCounter")->setInt32(st.tx_error_counter);
    info->getField(env->exec, "rxErrorCounter")->setInt32(st.rx_error_counter);
    
    // Set queue info (ESP32 doesn't provide these directly, use default values)
    info->getField(env->exec, "txQueueFree")->setInt32(32); // Default queue size
    info->getField(env->exec, "rxQueueFill")->setInt32(0);
    
    // Set state
    JClass *state_cls = env->findClass("flint/machine/CanStatus$State");
    FieldValue *state_field = info->getField(env->exec, "state");
    
    if (state_cls && state_field) {
        // Map TWAI states to Java enum
        const char* enum_name = "RUNNING";
        if (!ctx->is_started) {
            enum_name = "STOPPED";
        } else if (st.state == TWAI_STATE_BUS_OFF) {
            enum_name = "BUS_OFF";
        } else if (st.state == TWAI_STATE_RECOVERING) {
            enum_name = "RECOVERING";
        }
        
        // Get enum value
        FieldValue *enum_val = state_cls->getClassLoader()->getStaticField(env->exec, enum_name);
        if (enum_val) {
            state_field->setObj(enum_val->getObj());
        }
    }
    
    return info;
}

/* --------------------------------------------------------- */

jvoid NativeCan_ClearTxQueue(FNIEnv *env, jobject obj) {
    CANContext* ctx = get_context(env, obj);
    if (ctx && ctx->is_started) {
        // Stop and restart to clear TX queue
        twai_stop();
        twai_start();
    }
}

/* --------------------------------------------------------- */

jvoid NativeCan_ClearRxQueue(FNIEnv *env, jobject obj) {
    // Discard all received messages
    twai_message_t frame;
    while (twai_receive(&frame, 0) == ESP_OK) {
        // Just discard
    }
}