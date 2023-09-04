#include <stdio.h>
#include <stdbool.h>
#include "iot_globals.h"
#include "iot_config.h"
#include "drivers/iot_simple_switch.h"
#include "iot_nvs.h"
#include "iot_wifi.h"
#include "iot_mqtt.h"
#include "iot_defines.h"
#include "iot_httpd.h"
#include "iot_utils.h"


#include "dummy.h"
#include "iot_simple_switch.h"


void app_main(void)
{
    iot_init();
 /*
    iot_nvs_set_int_value(IOT_KEY_WIFI_MODE, 2);
    iot_nvs_set_str_value(IOT_KEY_WIFI_SSID, "schwefel");
    iot_nvs_set_str_value(IOT_KEY_WIFI_PASS, "0101020221");
    iot_nvs_set_str_value(IOT_KEY_MQTT_BROKER , "homeassistant.local");
*/
    
    if(gpio_get_level(GPIO_NUM_4)) {
        printf("GPIO 4 is held HI, loading dummy config....\n");
        iot_save_config(dummy_intiConfig());
    }
    
 
    if(iot_start_wifi()) {
        iot_start_httpd();
        iot_start_mqtt();
        iot_conf_controller(iot_open_config());
        spiffs_dir("/");
    }
 
/* 
    iot_intr_switch_simple_config_t* jasonTest = calloc(1, sizeof(iot_intr_switch_simple_config_t));
        jasonTest->intrTaskName = "Task_Inter_1";
        jasonTest->intrPin = GPIO_NUM_0;
        jasonTest->intrSimpleSwitchType = IOT_INTR_SWITCH_TOGGLE;
        jasonTest->outPin = GPIO_NUM_1;
        jasonTest->outPull = IOT_GPIO_PULL_UP;
        jasonTest->intrPull = IOT_GPIO_PULL_UP;
        //jasonTest->intrType = GPIO_INTR_ANYEDGE;
        jasonTest->inputInvert = true;
        jasonTest->timerDelay = 5000;
        jasonTest->mqttSubTopic = "/JSONTest";
        jasonTest->mqttDataOn = "On";
        jasonTest->mqttDataOff = "Off";



    iot_mqtt_subscribe_callback_t* callback = malloc(sizeof(iot_mqtt_subscribe_callback_t));
        callback->callbackData = (void*)jasonTest;
        callback->callbackFunc = &iot_mqtt_callback_prototype;

    char* harry = "/ESP-IOT-MQTT/JasonTest";

    //HT_ADD(mqttSubscribeMap, "/ESP-IOT-MQTT/JasonTest", callback);

    iot_mqtt_callback_add(harry, callback);

    //iot_mqtt_subscribe_callback_t* hollaback = (iot_mqtt_subscribe_callback_t*)HT_LOOKUP(mqttSubscribeMap, "/ESP-IOT-MQTT/JasonTest");
    iot_mqtt_subscribe_callback_t* hollaback = iot_mqtt_subscribe_get(harry);

    void (*tom)(void*) = hollaback->callbackFunc;
    void (*bob)(void*) = callback->callbackFunc;

    iot_intr_switch_simple_config_t* ttom = (iot_intr_switch_simple_config_t*)hollaback->callbackData;
    
    tom(ttom);

    bob(callback->callbackData);
    */
    return;
}



