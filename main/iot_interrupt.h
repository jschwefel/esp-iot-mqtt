/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#pragma once

#include <stdbool.h>
#include "driver/gpio.h"

void iot_config_gpio_interrupt(gpio_num_t intrzPin, void* intrISR);
esp_err_t iot_intr_pin_config(gpio_num_t gpioPin, gpio_mode_t mode, bool pullUp, bool pullDown, gpio_int_type_t intrType);
esp_err_t iot_io_pin_config(gpio_num_t gpioPin, gpio_mode_t mode, bool pullUp, bool pullDown);

esp_err_t iot_intr_gpio_setup(gpio_num_t intrPin, iot_gpio_pull_t intrPull, 
                                gpio_int_type_t intrType, void* intrISR,
                                gpio_num_t outPin, iot_gpio_pull_t outPull);
