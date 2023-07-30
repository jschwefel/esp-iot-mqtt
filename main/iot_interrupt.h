/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#pragma once

#include <stdbool.h>
#include "driver/gpio.h"

//void iot_config_gpio_interrupt(gpio_num_t intrzPin, void* intrISR);
//esp_err_t iot_intr_pin_config(gpio_num_t gpioPin, gpio_mode_t mode, bool pullUp, bool pullDown, gpio_int_type_t intrType);
//esp_err_t iot_io_pin_config(gpio_num_t gpioPin, gpio_mode_t mode, bool pullUp, bool pullDown);
esp_err_t iot_intr_gpio_setup(iot_intr_config_t intrConfig);
