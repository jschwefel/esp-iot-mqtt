/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#pragma once

#include <stdbool.h>
#include "driver/gpio.h"

//void iot_config_gpio_interrupt(gpio_num_t intrzPin, void* intrISR);
//esp_err_t iot_intr_pin_config(gpio_num_t gpioPin, gpio_mode_t mode, bool pullUp, bool pullDown, gpio_int_type_t intrType);
//esp_err_t iot_io_pin_config(gpio_num_t gpioPin, gpio_mode_t mode, bool pullUp, bool pullDown);
esp_err_t iot_intr_gpio_setup(iot_intr_switch_simple_config_t intrConfig);
/* 
void iot_intr_gpio_set_config(char* intrName,  
                                gpio_num_t intrPin,
                                iot_gpio_pull_t intrPull,
                                gpio_int_type_t intrType,
                                iot_simple_switch_type_t intrSwitchType,
                                int timeDeley,
                                gpio_num_t outPin,
                                iot_gpio_pull_t outPull,
                                bool inputInvert, 
                                char* mqttSubTopic,
                                char* mqttOn,
                                char* mqttOff
                                );
 */
void iot_intr_gpio_setup_config();
void iot_conf_controller(iot_config_linked_list_t* configList);
iot_config_linked_list_t* iot_open_config(void);
cJSON* serialize_iot_intr_switch_simple_config(void* configItemPtr);
iot_config_linked_list_t* deserialize_iot_intr_switch_simple_config(char* key, cJSON* configJson);
iot_config_linked_list_t* deserialize_dummy(char* key, cJSON* configJson);
iot_config_linked_list_t* iot_iot_settings_process_config_update_simple_switch(int sequence, char* queryString);