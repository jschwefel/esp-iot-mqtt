#pragma once

#include "iot_enums.h"
#include "iot_structs.h"
#include "driver/gpio.h"
#include "esp_wifi.h"

typedef struct {
    char* mqttTopic;
    iot_mqtt_qos_t mqttTopicQos;
    char* mqttSubscribe;
    iot_mqtt_qos_t mqttSubscribeQos;
} iot_mqtt_config_t;

// GPIO Simple Swith Interrupt
typedef struct {
    char* intrTaskName;
    gpio_num_t intrPin;
    bool inputInvert;
    iot_gpio_pull_t intrPull;
    gpio_int_type_t intrType; 
    iot_simple_switch_type_t intrSimpleSwitchType;
    gpio_num_t outPin;
    iot_gpio_pull_t outPull;
    uint32_t timerDelay;
    iot_mqtt_config_t* mqttConfig;
    char* mqttDataOn;
    char* mqttDataOff; 
} iot_intr_switch_simple_config_t;

typedef struct {
    iot_config_item_type_t configItemType;
    char* configKey;
    void* configItem;
} iot_config_item_t;

typedef struct {
    iot_config_item_t* configEntry;
    void* next;
} iot_config_linked_list_t;

typedef struct {
    char* topic;
    char* data;
    int qos;
    int retain;
} iot_mqtt_message_t;


typedef struct {  
    int32_t mode;
    char* ssid;
    char* passwd;
} iot_wifi_conf_t;


typedef struct {
    void* callbackData;
    void* callbackFunc;
} iot_mqtt_subscribe_callback_t;



typedef struct {
    char* intrTaskName;
    char* mqttSubTopic;
    char* mqttDataOn;
    char* mqttDataOff;
    gpio_num_t intrPin;
    iot_gpio_pull_t intrPull;
    gpio_int_type_t intrType; 
    iot_simple_switch_type_t intrSimpleSwitchType;
    gpio_num_t outPin;
    iot_gpio_pull_t outPull;
    uint32_t timerDelay;
    bool inputInvert;
} dummy_config_t;

