/* This file was automatically generated.  Do not edit! */

#undef INTERFACE
#pragma once


#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "mqtt_client.h"

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
    bool outInvert;
    char* mqttSubTopic;
    char* mqttDataOn;
    char* mqttDataOff;
} iot_isr_config_t;

typedef struct {
    char* intrTaskName;
    gpio_num_t intrPin;
    iot_gpio_pull_t intrPull;
    gpio_int_type_t intrType; 
    iot_switch_type_t intrSwitchType;
    gpio_num_t outPin;
    iot_gpio_pull_t outPull;
    uint32_t timerDelay;
    iot_isr_config_t intrISR;
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

extern QueueHandle_t gpioInterruptQueue;
extern QueueHandle_t gpioTimerInterruptQueue;

extern nvs_handle_t iot_nvs_user_handle;
extern iot_configuration_s iot_configuration;
extern const char *TAG;
extern esp_mqtt_client_handle_t mqtt_client;
extern char* baseTopic;


void globals_init(void);