#include "iot_globals.h"
#include "iot_nvs.h"
#include "nvs_flash.h"




const char *TAG = "ESP-IOT-MQTT";

iot_configuration_s iot_configuration;
nvs_handle_t iot_nvs_user_handle;



void globals_init(void)
{
    iot_nvs_user_handle = iot_init_flash(iot_nvs_user_handle, "configuration");

    //handle = iot_nvs_user_handle;

    //mac_half_low = get_mac_address_half_low();
};
