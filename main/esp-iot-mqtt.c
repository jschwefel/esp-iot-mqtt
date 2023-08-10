#include <stdio.h>
#include <stdbool.h>
#include "iot_enums.h"
#include "iot_structs.h"
#include "iot_enums.h"
#include "iot_globals.h"
#include "iot_config.h"
#include "iot_interrupt.h"
#include "esp_log.h"
#include "dummy.h"
#include "iot_nvs.h"
#include "iot_defines.h"
#include "iot_wifi.h"
#include "iot_mqtt.h"




void app_main(void)
{
    iot_init();

    //iot_nvs_set_int_value(IOT_KEY_WIFI_MODE, 2);
    //iot_nvs_set_str_value(IOT_KEY_WIFI_SSID, "SSID HERE");
    //iot_nvs_set_str_value(IOT_KEY_WIFI_PASS, "PASSWORD HERE");

    iot_save_config(dummy_intiConfig());


    iot_start_wifi();
    iot_start_mqtt();
    iot_conf_controller(iot_open_config());
    return;
}



