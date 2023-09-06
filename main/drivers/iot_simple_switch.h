/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#pragma once

#include <stdbool.h>
#include "mqtt_client.h"
#include "driver/gpio.h"
#include "../iot_defines.h"
#include "../iot_enums.h"
#include "../iot_structs.h"

esp_err_t iot_intr_gpio_setup(iot_intr_switch_simple_config_t intrConfig);
void iot_intr_gpio_setup_config();
void iot_conf_controller(iot_config_linked_list_t* configList);
iot_config_linked_list_t* iot_open_config(void);
cJSON* serialize_iot_intr_switch_simple_config(void* configItemPtr);
iot_config_linked_list_t* deserialize_iot_intr_switch_simple_config(char* key, cJSON* configJson);
iot_config_linked_list_t* deserialize_dummy(char* key, cJSON* configJson);
iot_config_linked_list_t* iot_iot_settings_process_config_update_simple_switch(int sequence, char* queryString);
void simple_switch_mqtt_subscribe_handler(void* data, esp_mqtt_event_t* event);