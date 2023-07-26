/* This file was automatically generated.  Do not edit! */

#undef INTERFACE
#pragma once


#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

typedef int (*iotVoidFuncPtr)(void*);

typedef enum
{
    IOT_GPIO_PULL_UP =      0,
    IOT_GPIO_PULL_DOWN =    1,
    IOT_GPIO_PULL_BOTH =    2,
} iot_gpio_pull_t;

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
    gpio_num_t intrPin;
    gpio_num_t outPin;
    bool outInvet;
    char* mqttSubTopic;
    char* mqttDataHigh;
    char* mqttDataLow;
    iotVoidFuncPtr intrFunc;
} iot_intr_params_t;

extern QueueHandle_t interruptQueue;
extern QueueHandle_t interruptQueue2;
extern nvs_handle_t iot_nvs_user_handle;
extern iot_configuration_s iot_configuration;
extern const char *TAG;


void globals_init(void);