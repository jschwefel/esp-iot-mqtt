#include "iot_globals.h"
#include "iot_nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
//#include "freertos/task.h"




const char *TAG = "ESP-IOT-MQTT";

iot_configuration_s iot_configuration;
nvs_handle_t iot_nvs_user_handle;
QueueHandle_t interruptQueue;
QueueHandle_t interruptQueue2;



void globals_init(void)
{
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3));
    iot_nvs_user_handle = iot_init_flash(iot_nvs_user_handle, "configuration");
    interruptQueue = xQueueCreate(10, sizeof(iot_isr_params_t));

    //handle = iot_nvs_user_handle;

    //mac_half_low = get_mac_address_half_low();
};
