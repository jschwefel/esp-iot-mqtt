#include "cJSON.h"
#include "iot_structs.h"
#include "nvs_flash.h"
#include "iot_nvs.h"
#include "mqtt_client.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

const char* TAG = "ESP-IOT-MQTT";

//Need top convert this to configurable.
const char* baseTopic = "/ESP-IOT-MQTT";

cJSON* iotConfiguration;
nvs_handle_t iot_nvs_user_handle;
QueueHandle_t simpleSwitchInreQueue;
esp_mqtt_client_handle_t iotMqttClient;
iot_wifi_conf_t iot_wifi_conf;

void iot_init(void) {
    iotConfiguration = cJSON_CreateObject();
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3));
    iot_nvs_user_handle = iot_init_flash(iot_nvs_user_handle, "configuration");
    simpleSwitchInreQueue = xQueueCreate(10, sizeof(iot_intr_switch_simple_config_t));

}
