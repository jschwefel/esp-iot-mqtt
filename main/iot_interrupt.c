#include "iot_globals.h"
#include "iot_value_defines.h"
#include "iot_mqtt.h"
#include "iot_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/gpio_filter.h"
#include "iot_interrupt.h"

//#include "hal/gpio_ll.h"
//#include "esp_sleep.h"
#include "esp_log.h"



static void iot_gpio_isr_task_queue_handler(void *params);
static void iot_gpio_isr_intr_handler(void* arg);
static void iot_intr_switch_toggle(iot_isr_params_t *intrParams);



static void  iot_gpio_isr_intr_handler(void* arg) // gpio_isr_handler_add
{
  	iot_isr_params_t* intrParams = (iot_isr_params_t*) arg;
	//gpio = GPIO_NUM_22;
	xQueueSendToBackFromISR(interruptQueue, intrParams, NULL);
}

static void iot_intr_switch_toggle(iot_isr_params_t *intrParams)
{
    // The toggle switch type is used for On/Off type switches.
    // This type of switch will trigger on 'GPIO_INTR_ANYEDGE'.
    // Typical uses are door and window open/close switches.
    
    // Check to see if there is an outpit GPIO (state
    // indicator) assigned in the interrupt config.
    bool indicator = intrParams->outPin != GPIO_NUM_NC ? true : false;

    // Check to see if the input in inverted. Inverted input are
    // used when the sensor uses an "Acrive Low" output.
    bool intrPin = gpio_normailized_state(intrParams->outInvert, intrParams->intrPin);

    if(intrPin) {
        iot_send_mqtt(intrParams->mqttSubTopic, intrParams->mqttDataOn);
    } else {
        iot_send_mqtt(intrParams->mqttSubTopic, intrParams->mqttDataOff);
    }

    if(indicator) { gpio_set_level(intrParams->outPin, intrPin); }

}


static void iot_intr_switch_one_shot(iot_isr_params_t *intrParams)
{
    // The toggle switch type is used for On/Off type switches.
    // This type of switch will trigger on 'GPIO_INTR_ANYEDGE'.
    // Typical uses are door and window open/close switches.
    
    // Check to see if there is an outpit GPIO (state
    // indicator) assigned in the interrupt config.
    bool indicator = intrParams->outPin != GPIO_NUM_NC ? true : false;

    // Check to see if the input in inverted. Inverted input are
    // used when the sensor uses an "Acrive Low" output.
    bool intrPin = gpio_normailized_state(intrParams->outInvert, intrParams->intrPin);

    if(intrPin) {
        iot_send_mqtt(intrParams->mqttSubTopic, intrParams->mqttDataOn);
    } else {
        iot_send_mqtt(intrParams->mqttSubTopic, intrParams->mqttDataOff);
    }

    if(indicator) { gpio_set_level(intrParams->outPin, intrPin); }

}


static void iot_gpio_isr_task_queue_handler(void *params) // xTaskCreate
{
    while(true) {
        iot_isr_params_t* intrParams = malloc(sizeof(iot_isr_params_t));
        ESP_LOGI(TAG, "Waiting on interrupt queue");
		BaseType_t rc = xQueueReceive(interruptQueue, intrParams, portMAX_DELAY);

        switch(intrParams->intrSwitchType) {
            case IOT_ISR_SWITCH_ONE_SHOT :
                if (intrParams->outPin != GPIO_NUM_NC) {
                    
                    gpio_set_level(intrParams->outPin, GPIO_OUT_LEVEL_LOW);
                }
                break;
            
            case IOT_ISR_SWITCH_TOGGLE :
                iot_intr_switch_toggle(intrParams);
                break;

            case IOT_ISR_SWITCH_TIMER :
                break;
        }
		ESP_LOGI(TAG, "Woke from interrupt queue wait: %d on GPIO: %i", rc, intrParams->intrPin);
    }
}


esp_err_t iot_intr_gpio_setup(iot_intr_config_t intrConfig)

{
    // Confiure pins.

    // Pretty sure there is a better way. But for now, it works.
    bool intrPullUp = ((intrConfig.intrPull == 0) || (intrConfig.intrPull == 2)) ? true : false;
    bool intrPullDown = ((intrConfig.intrPull == 1) || (intrConfig.intrPull == 2)) ? true : false;
    bool outPullUp = ((intrConfig.outPull == 0) || (intrConfig.intrPull == 2)) ? true : false;
    bool outPullDown = ((intrConfig.outPull == 1) || (intrConfig.intrPull == 2)) ? true : false;



    gpio_config_t intrPinConfig = {
        .pin_bit_mask = BIT64(intrConfig.intrPin),
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = intrPullDown,
        .pull_up_en = intrPullUp,
        .intr_type = intrConfig.intrType,
    };
    ESP_ERROR_CHECK(gpio_config(&intrPinConfig));

    if (intrConfig.outPin != GPIO_NUM_NC) {
        gpio_config_t outPinConfig = {
            .pin_bit_mask = BIT64(intrConfig.outPin),
            .mode = GPIO_MODE_OUTPUT,
            .pull_down_en = outPullDown,
            .pull_up_en = outPullUp,
            .intr_type = GPIO_MODE_DEF_DISABLE,
        };
        ESP_ERROR_CHECK(gpio_config(&outPinConfig));
    }


    // Configure GLitch Filter on interrupt pin.

    gpio_pin_glitch_filter_config_t* glitchFilterConfig = malloc(sizeof(gpio_pin_glitch_filter_config_t));
    glitchFilterConfig->clk_src = 4; // SOC_MOD_CLK_PLL_F80M
    glitchFilterConfig->gpio_num = intrConfig.intrPin;
    gpio_glitch_filter_handle_t* glitchFilter = malloc(sizeof(gpio_glitch_filter_handle_t));
    ESP_ERROR_CHECK(gpio_new_pin_glitch_filter(glitchFilterConfig, glitchFilter));
    ESP_ERROR_CHECK(gpio_glitch_filter_enable(*glitchFilter));

    // Configure the interrupt

    iot_isr_params_t* intrParams = malloc(sizeof(iot_isr_params_t));
    //intrParams->intrFunc = NULL;
    intrParams->intrPin = intrConfig.intrPin;
    intrParams->outPin = intrConfig.outPin;
    intrParams->outInvert = intrConfig.intrISR.outInvert;
    intrParams->intrSwitchType = intrConfig.intrType == GPIO_INTR_ANYEDGE ? IOT_ISR_SWITCH_TOGGLE : IOT_ISR_SWITCH_ONE_SHOT;
    intrParams->mqttSubTopic = intrConfig.intrISR.mqttSubTopic;
    intrParams->mqttDataOn = intrConfig.intrISR.mqttDataOn;
    intrParams->mqttDataOff = intrConfig.intrISR.mqttDataOff;

    
    gpio_isr_handler_add((uint32_t)intrParams->intrPin, iot_gpio_isr_intr_handler, intrParams);
    
    
    xTaskCreate(iot_gpio_isr_task_queue_handler, intrConfig.intrTaskName, 2048, intrParams, 1, NULL);
    
    
    
    return ESP_OK;


}