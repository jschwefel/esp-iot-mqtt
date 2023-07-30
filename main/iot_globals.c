#include "iot_globals.h"
#include "iot_nvs.h"
#include "iot_utils.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
//#include "freertos/task.h"
#include "mqtt_client.h"
//#include "driver/timer.h"




const char *TAG = "ESP-IOT-MQTT";

iot_configuration_s iot_configuration;
nvs_handle_t iot_nvs_user_handle;
QueueHandle_t gpioInterruptQueue;
QueueHandle_t gpioTimerInterruptQueue;
QueueHandle_t mqttQueue;
esp_mqtt_client_handle_t iotMqttClient;
char* baseTopic;


void globals_init(void)
{
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3));
    
    iot_nvs_user_handle = iot_init_flash(iot_nvs_user_handle, "configuration");
    gpioInterruptQueue = xQueueCreate(10, sizeof(iot_isr_params_t));
    gpioTimerInterruptQueue = xQueueCreate(10, sizeof(iot_isr_params_t));
    mqttQueue = xQueueCreate(50, sizeof(int));

    // Need to make configurable
    char* base = "iot-";
    char* lastMac = get_mac_address_half_low();
    baseTopic = concat(base, lastMac);


    //handle = iot_nvs_user_handle;

    //mac_half_low = get_mac_address_half_low();
};
