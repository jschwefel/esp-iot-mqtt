#pragma once

#include "iot_enums.h"
#include "iot_structs.h"
#include "driver/gpio.h"
#include "esp_wifi.h"

// GPIO Simple Swith Interrupt
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
    bool outInvert;
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
} wifi_connection_t;

typedef struct {
    wifi_connection_t wifi_settings;
} iot_wifi_conf_t;





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
    bool outInvert;
} dummy_config_t;

