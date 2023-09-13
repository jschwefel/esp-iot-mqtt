#include "cJSON.h"
#include "esp_log.h"
#include "iot_structs.h"
#include "nvs_flash.h"
#include "driver/rmt_tx.h"
#include "iot_nvs.h"
#include "iot_defines.h"
#include "mqtt_client.h"
#include "drivers/iot_stepper_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "../Hash-Table/hashtable.h"

const char* TAG = "ESP-IOT-MQTT";

//Need top convert this to configurable.
const char* baseTopic = "ESP-IOT-MQTT/";

//cJSON* iotConfiguration;
nvs_handle_t iot_nvs_user_handle;
QueueHandle_t simpleSwitchInreQueue;
QueueHandle_t stepperLimitIntrQueue;
esp_mqtt_client_handle_t iotMqttClient;
iot_wifi_conf_t iot_wifi_conf;
hash_table_t* mqttSubscribeMap;
rmt_encoder_handle_t uniformMotorEncoder;


//For global storage of data. Sure there is better way, but for now...
int iot_gpio_array[GPIO_NUM_MAX];

void iot_init(void) {
    //iotConfiguration = cJSON_CreateObject();
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3));
    iot_nvs_user_handle = iot_init_flash(iot_nvs_user_handle, "configuration");
    stepperLimitIntrQueue = xQueueCreate(2, sizeof(rmt_channel_handle_t));
    simpleSwitchInreQueue = xQueueCreate(10, sizeof(iot_intr_switch_simple_config_t));
    esp_log_level_set(TAG, ESP_LOG_DEBUG);
    //mqttSubscribeMap = hash_table_new(MODE_VALUEREF);
    mqttSubscribeMap = hash_table_new(MODE_COPY);
    stepper_motor_uniform_encoder_config_t uniform_encoder_config = {
        .resolution = STEP_MOTOR_RESOLUTION_HZ,
    };
    ESP_ERROR_CHECK(rmt_new_stepper_motor_uniform_encoder(&uniform_encoder_config, &uniformMotorEncoder));
}
