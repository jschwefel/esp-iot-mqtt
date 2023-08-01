#include "iot_globals.h"
#include "iot_gpio_interrupt.h"
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
#include "iot_config.h"
//#include "sdkconfig.h"


#include "benhoyt_hashtable.h"


void app_main(void)
{
    globals_init();

     
    init_wifi();
    iot_start_mqtt();










































    iot_intr_gpio_set_config("Task 0", GPIO_NUM_0, IOT_GPIO_PULL_UP,
                            GPIO_INTR_ANYEDGE, IOT_ISR_SWITCH_TOGGLE, 0,
                            GPIO_NUM_1, IOT_GPIO_PULL_DOWN, 
                            true, "/PantrySwitc0", "On", "Off");

    iot_intr_gpio_set_config("Task 2", GPIO_NUM_2, IOT_GPIO_PULL_DOWN,
                            GPIO_INTR_POSEDGE, IOT_ISR_SWITCH_ONE_SHOT, 5000,
                            GPIO_NUM_3, IOT_GPIO_PULL_DOWN, 
                            true, "/PantrySwitch2", "OneShot", NULL);

    iot_intr_gpio_set_config("Task 4", GPIO_NUM_4, IOT_GPIO_PULL_DOWN,
                            GPIO_INTR_POSEDGE, IOT_ISR_SWITCH_TIMER, 5000,
                            GPIO_NUM_5, IOT_GPIO_PULL_DOWN, 
                            true, "/PantrySwitch4", "On", "Off");

    iot_intr_gpio_set_config("Task 6", GPIO_NUM_6, IOT_GPIO_PULL_DOWN,
                            GPIO_INTR_POSEDGE, IOT_ISR_SWITCH_TOGGLE, 0,
                            GPIO_NUM_7, IOT_GPIO_PULL_DOWN, true,
                            "/PantrySwitch6", "On", "Off");
    
    iot_intr_gpio_set_config("Task 8", GPIO_NUM_8, IOT_GPIO_PULL_DOWN,
                            GPIO_INTR_POSEDGE, IOT_ISR_SWITCH_ONE_SHOT, 0,
                            GPIO_NUM_9, IOT_GPIO_PULL_DOWN, false,
                            "/PantrySwitch8","On", NULL);

    iot_intr_gpio_setup_config();




   
}










