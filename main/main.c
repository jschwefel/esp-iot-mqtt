#include "iot_globals.h"
#include "iot_interrupt.h"
#include "iot_wifi.h"
#include "iot_mqtt.h"
/*
 * Test interrupt handling on a GPIO.
 * In this fragment we watch for a change on the input signal
 * of GPIO 25.  When it goes high, an interrupt is raised which
 * adds a message to a queue which causes a task that is blocking
 * on the queue to wake up and process the interrupt.
 */
#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "sdkconfig.h"

void old_app_main(void *ignore);





void app_main(void)
{
    globals_init();
    init_wifi();
    iot_start_mqtt();
    
    iot_intr_config_t intr_gpio_0 = {
        .intrTaskName = "Task 0",
        .intrPin = GPIO_NUM_0,
        .intrPull = IOT_GPIO_PULL_UP,
        .intrType = GPIO_INTR_ANYEDGE,
        .outPin = GPIO_NUM_1,
        .intrISR = {
            .outInvert = true,
            .mqttSubTopic = "/PantrySwitch0",
            .mqttDataOn = "On",
            .mqttDataOff = "Off",
        }
    };
    
    iot_intr_config_t intr_gpio_2 = {
        .intrTaskName = "Task 2",
        .intrPin = GPIO_NUM_2,
        .intrPull = IOT_GPIO_PULL_DOWN,
        .intrType = GPIO_INTR_POSEDGE,
        .intrSwitchType = IOT_ISR_SWITCH_ONE_SHOT,
        .outPin = GPIO_NUM_3,
        .intrISR = {
            .outInvert = true,
            .mqttSubTopic = "/PantrySwitch1",
            .mqttDataOn = "ButtonPushed",
//            .mqttDataOff = "On",
        }
    };

    iot_intr_config_t intr_gpio_4 = {
        .intrTaskName = "Task 4",
        .intrPin = GPIO_NUM_4,
        .intrPull = IOT_GPIO_PULL_DOWN,
        .intrType = GPIO_INTR_POSEDGE,
        .intrSwitchType = IOT_ISR_SWITCH_TIMER,
        .timerDelay = 5000,
        .outPin = GPIO_NUM_5,
        .intrISR = {
            .outInvert = true,
            .mqttSubTopic = "/PantrySwitch2",
            .mqttDataOn = "On",
            .mqttDataOff = "Off",
        }
    };

    
    
    iot_intr_gpio_setup(intr_gpio_0);
    iot_intr_gpio_setup(intr_gpio_2);
    iot_intr_gpio_setup(intr_gpio_4);
    
    
    
    

}










