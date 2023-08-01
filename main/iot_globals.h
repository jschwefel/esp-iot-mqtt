/* This file was automatically generated.  Do not edit! */

#undef INTERFACE
#pragma once


#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "mqtt_client.h"
#include "benhoyt_hashtable.h"


typedef void (*iotVoidFuncPtr)(void*);

typedef enum 
{
    GPIO_OUT_LEVEL_LOW =        0,
    GPIO_OUT_LEVEL_HIGH =       1,
} gpio_out_level_t;

typedef enum
{
    IOT_GPIO_PULL_UP =          0,
    IOT_GPIO_PULL_DOWN =        1,
    IOT_GPIO_PULL_BOTH =        2,
} iot_gpio_pull_t;

typedef enum
{
    IOT_ISR_SWITCH_TOGGLE =     0,
    IOT_ISR_SWITCH_ONE_SHOT =   1,
    IOT_ISR_SWITCH_TIMER =      2,
} iot_switch_type_t;

typedef enum {
    IOT_CONFIG_SIMPLE_SWITCH =  0,
} iot_config_item_type_t;

typedef struct wifi_connection_s
{  
    int32_t mode;
    char* ssid;
    char* passwd;
} wifi_connection_s;

typedef struct iot_configuration_s
{
    wifi_connection_s wifi_settings;
} iot_configuration_s;

typedef struct {
    char* topic;
    char* data;
    int qos;
    int retain;
} iot_mqtt_message_t;

typedef struct {
    char* intrTaskName;
    char* mqttSubTopic;
    char* mqttDataOn;
    char* mqttDataOff;
    gpio_num_t intrPin;
    iot_gpio_pull_t intrPull;
    gpio_int_type_t intrType; 
    iot_switch_type_t intrSwitchType;
    gpio_num_t outPin;
    iot_gpio_pull_t outPull;
    uint32_t timerDelay;
    bool outInvert;

} iot_intr_config_t;

typedef struct {
    gpio_num_t intrPin;
    gpio_num_t outPin;
    iot_switch_type_t intrSwitchType;
    uint32_t timerDelay;
    bool outInvert;
    char* mqttSubTopic;
    char* mqttDataOn;
    char* mqttDataOff;
    //iotVoidFuncPtr intrFunc;  // iot_gpio_intr_pin
} iot_isr_params_t;


typedef struct {
    iot_config_item_type_t configItemType;
    char* configKey;
    void* configItem;
} iot_config_item_t;

typedef struct {
    iot_config_item_t* configEntry;
    void* next;
} iot_config_linked_list_t;

extern QueueHandle_t gpioInterruptQueue;
extern QueueHandle_t gpioTimerInterruptQueue;
extern QueueHandle_t mqttQueue;

extern nvs_handle_t iot_nvs_user_handle;
extern iot_configuration_s iot_configuration;
extern const char *TAG;
extern esp_mqtt_client_handle_t iotMqttClient;
extern char* baseTopic;
extern ht* iotConfigHashTable;


void globals_init(void);