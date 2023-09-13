#include <stdio.h>
#include <stdbool.h>
#include "iot_globals.h"
#include "iot_config.h"
#include "drivers/iot_simple_switch.h"
#include "drivers/iot_stepper_bipolar.h"
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

    iot_nvs_set_int_value(IOT_KEY_WIFI_MODE, 2);
    iot_nvs_set_str_value(IOT_KEY_WIFI_SSID, "schwefel");
    iot_nvs_set_str_value(IOT_KEY_WIFI_PASS, "0101020221");
    //iot_nvs_set_str_value(IOT_KEY_MQTT_BROKER , "homeassistant.local");
    iot_nvs_set_str_value(IOT_KEY_MQTT_BROKER , "172.31.1.132");

    
    //iot_nvs_set_blobstr_value(IOT_CONFIG_KEY, "");

    if(gpio_get_level(GPIO_NUM_4)) {
        printf("GPIO 4 is held HI, loading dummy config....\n");
        iot_save_config(dummy_intiConfig());
    }
    
 
    if(iot_start_wifi()) {
        iot_start_httpd();
        iot_start_mqtt();
        //iot_conf_controller(iot_open_config());
        iot_stepper_config_t* stepper = calloc(1, sizeof(iot_stepper_config_t));
            stepper->stepperTaskName = "Stepper_Task1";
            stepper->stepperDriver = IOT_STEPPER_A4988;
            stepper->pinStep = 10;
            stepper->pinEnable = 4;
            stepper->pinDirection = 11;
            stepper->pinMicroStep0 = 7;
            stepper->pinMicroStep1 = 6;
            stepper->pinMicroStep2 = 5;
            stepper->limitCW = GPIO_NUM_22;
            stepper->limitCCW = GPIO_NUM_23;
            stepper->reverse = false;
            stepper->mqttConfig = malloc(sizeof(iot_mqtt_config_t));
                stepper->mqttConfig->mqttSubscribe = "stepper1/control";
                stepper->mqttConfig->mqttSubscribeQos = IOT_MQTT_QOS_LEAST_ONCE;
                stepper->mqttConfig->mqttTopic = "stepper1/error";
                stepper->mqttConfig->mqttTopicQos = IOT_MQTT_QOS_LEAST_ONCE;
        
        iot_mqtt_configure(stepper);

/* Need to figure out why I cannot set up two RMT TX channels. 
        iot_stepper_config_t* stepper2 = calloc(1, sizeof(iot_stepper_config_t));
            stepper2->stepperTaskName = "Stepper_Task1";
            stepper2->stepperDriver = IOT_STEPPER_A4988;
            stepper2->pinStep = 0;
            stepper2->pinEnable = 2;
            stepper2->pinDirection = 14;
            stepper2->pinMicroStep0 = 6;
            stepper2->pinMicroStep1 = 9;
            stepper2->pinMicroStep2 = 1;
            stepper2->limitCW = GPIO_NUM_NC;
            stepper2->limitCCW = GPIO_NUM_NC;
            stepper2->reverse = false;
            stepper2->mqttConfig = malloc(sizeof(iot_mqtt_config_t));
                stepper2->mqttConfig->mqttSubscribe = "stepper2/control/#";
                stepper2->mqttConfig->mqttSubscribeQos = IOT_MQTT_QOS_LEAST_ONCE;
                stepper2->mqttConfig->mqttTopic = "stepper2/error";
                stepper2->mqttConfig->mqttTopicQos = IOT_MQTT_QOS_LEAST_ONCE;
        
        iot_mqtt_configure(stepper2);
*/
 


        spiffs_dir("/");
    }
 
    return;
}



