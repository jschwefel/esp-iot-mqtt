/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#pragma once


#include "nvs_flash.h"




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

extern nvs_handle_t iot_nvs_user_handle;
extern iot_configuration_s iot_configuration;
extern const char *TAG;


void globals_init(void);