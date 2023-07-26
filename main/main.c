#include <stdio.h>
#include <stdbool.h>
#include "iot_globals.h"
#include "iot_wifi.h"
#include "iot_interrupt.h"
#include "driver/gpio.h"
#include "iot_interrupt.h"
#include "esp_log.h"



void app_main(void)
{
    globals_init();
    //init_wifi();
    iot_intr_config_t intr_gpio_0 = {
        .intrTaskName = "Task 0",
        .intrPin = GPIO_NUM_0,
        .intrPull = IOT_GPIO_PULL_UP,
        .intrType = GPIO_INTR_NEGEDGE,
        .outPin = GPIO_NUM_1,
        .intrISR = {
            .outInvert = true,
            .mqttSubTopic = "/PantrySwitch0",
            .mqttDataHigh = "Off",
            .mqttDataLow = "On",
        }
    };
    
    iot_intr_config_t intr_gpio_2 = {
        .intrTaskName = "Task 2",
        .intrPin = GPIO_NUM_2,
        .intrPull = IOT_GPIO_PULL_DOWN,
        .intrType = GPIO_INTR_POSEDGE,
        .outPin = GPIO_NUM_3,
        .intrISR = {
            .outInvert = true,
            .mqttSubTopic = "/PantrySwitch1",
            .mqttDataHigh = "Off",
            .mqttDataLow = "On",
        }
    };

    iot_intr_config_t intr_gpio_4 = {
        .intrTaskName = "Task 4",
        .intrPin = GPIO_NUM_4,
        .intrPull = IOT_GPIO_PULL_DOWN,
        .intrType = GPIO_INTR_POSEDGE,
        .outPin = GPIO_NUM_5,
        .intrISR = {
            .outInvert = true,
            .mqttSubTopic = "/PantrySwitch1",
            .mqttDataHigh = "Off",
            .mqttDataLow = "On",
        }
    };


    iot_intr_gpio_setup(intr_gpio_0);
    iot_intr_gpio_setup(intr_gpio_2);
    iot_intr_gpio_setup(intr_gpio_4);






//    iot_intr_pin_config(GPIO_NUM_2, GPIO_MODE_INPUT, true, false, GPIO_INTR_ANYEDGE);
//    iot_io_pin_config(GPIO_NUM_3, GPIO_MODE_OUTPUT, false, true);
//    iot_config_gpio_interrupt(GPIO_NUM_2, iot_gpio_intr_pin2);


    return;
}
