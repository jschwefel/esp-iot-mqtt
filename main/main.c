#include "iot_globals.h"
#include "iot_interrupt.h"
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
    
    //ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3));
    

    //init_wifi();
    iot_intr_config_t intr_gpio_0 = {
        .intrTaskName = "Task 0",
        .intrPin = GPIO_NUM_0,
        .intrPull = IOT_GPIO_PULL_UP,
        .intrType = GPIO_INTR_ANYEDGE,
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
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    //old_app_main(NULL);
}














static char tag[] = "test_intr";
static QueueHandle_t q1;


#define TEST_GPIO (0) 
static void old_iot_isr_handler(void *args) { //  iot_isr_intr_handler
	gpio_num_t gpio;
	gpio = TEST_GPIO;
	xQueueSendToBackFromISR(q1, &gpio, NULL);
}

void old_interrupt_handler(void *args) 
{   
    gpio_num_t gpio;

	while(1) {
		ESP_LOGI(tag, "Waiting on interrupt queue");
		BaseType_t rc = xQueueReceive(q1, &gpio, portMAX_DELAY);
		ESP_LOGI(tag, "Woke from interrupt queue wait: %d", rc);
	}
}

void old_app_main(void *ignore) {
	ESP_LOGI(tag, ">> test1_task");
	gpio_num_t gpio;
	q1 = xQueueCreate(10, sizeof(gpio_num_t));

	gpio_config_t gpioConfig;
	gpioConfig.pin_bit_mask = BIT64(TEST_GPIO);
	gpioConfig.mode         = GPIO_MODE_INPUT;
	gpioConfig.pull_up_en   = GPIO_PULLUP_ENABLE;
	gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
	gpioConfig.intr_type    = GPIO_INTR_ANYEDGE;
	gpio_config(&gpioConfig);

	gpio_install_isr_service(0);
	gpio_isr_handler_add(TEST_GPIO, old_iot_isr_handler, NULL);
    xTaskCreate(old_interrupt_handler, "Task 0", 2048, NULL, 1, NULL);
//	while(1) {
//		ESP_LOGI(tag, "Waiting on interrupt queue");
//		BaseType_t rc = xQueueReceive(q1, &gpio, portMAX_DELAY);
//		ESP_LOGI(tag, "Woke from interrupt queue wait: %d", rc);
//	}
//	vTaskDelete(NULL);

}