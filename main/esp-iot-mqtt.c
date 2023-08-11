#include <stdio.h>
#include <stdbool.h>
#include "iot_globals.h"
#include "iot_config.h"
#include "iot_interrupt.h"
#include "iot_nvs.h"
#include "iot_wifi.h"
#include "iot_mqtt.h"
#include "iot_defines.h"


#include "dummy.h"



void app_main(void)
{
    iot_init();

    iot_nvs_set_int_value(IOT_KEY_WIFI_MODE, 2);
    iot_nvs_set_str_value(IOT_KEY_WIFI_SSID, "schwefel");
    iot_nvs_set_str_value(IOT_KEY_WIFI_PASS, "0101020221");
    //iot_save_config(dummy_intiConfig());


    iot_start_wifi();
    iot_start_mqtt();
    iot_conf_controller(iot_open_config());
    return;
}



