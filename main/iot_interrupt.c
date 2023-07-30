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
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "driver/gpio_filter.h"
#include "iot_interrupt.h"
#include "freertos/portmacro.h"
#include "esp_log.h"



TimerHandle_t timerHandle;

static void iot_gpio_isr_task_queue_handler(void *params);
//static void iot_gpio_isr_timer_task_queue_handler(void *params); // xTaskCreate
static void iot_gpio_isr_intr_handler(void* arg);
//static void iot_gpio_timer_isr_intr_handler(void* arg); // gpio_isr_handler_add
static void iot_intr_switch_toggle(iot_isr_params_t *intrParams);
static void iot_intr_switch_one_shot_and_timer(iot_isr_params_t *intrParams);
static void gpio_timer_intr_callback(TimerHandle_t timer);


static void  iot_gpio_isr_intr_handler(void* arg) // gpio_isr_handler_add
{
  	iot_isr_params_t* intrParams = (iot_isr_params_t*) arg;
    gpio_intr_disable(intrParams->intrPin);
	xQueueSendToBackFromISR(gpioInterruptQueue, intrParams, NULL);
    gpio_intr_enable(intrParams->intrPin);
}

static void iot_gpio_isr_task_queue_handler(void *params) // xTaskCreate
{
    while(true) {
        iot_isr_params_t* intrParams = malloc(sizeof(iot_isr_params_t));
        ESP_LOGI(TAG, "Waiting on interrupt queue");
		BaseType_t rc = xQueueReceive(gpioInterruptQueue, intrParams, portMAX_DELAY);

        switch(intrParams->intrSwitchType) {
            case IOT_ISR_SWITCH_ONE_SHOT :
            case IOT_ISR_SWITCH_TIMER :
                iot_intr_switch_one_shot_and_timer(intrParams);
                break;
            
            case IOT_ISR_SWITCH_TOGGLE :
                iot_intr_switch_toggle(intrParams);
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

    // Configure the interrupt parameters var
    iot_isr_params_t* intrParams = malloc(sizeof(iot_isr_params_t));
    intrParams->intrPin = intrConfig.intrPin;
    intrParams->outPin = intrConfig.outPin;
    intrParams->outInvert = intrConfig.outInvert;
    intrParams->intrSwitchType = intrConfig.intrSwitchType;
    intrParams->timerDelay = intrConfig.intrSwitchType == IOT_ISR_SWITCH_ONE_SHOT ? IOT_ISR_ONE_SHOT_OUT_DELAY : intrConfig.timerDelay;
    intrParams->mqttSubTopic = intrConfig.mqttSubTopic;
    intrParams->mqttDataOn = intrConfig.mqttDataOn;
    intrParams->mqttDataOff = intrConfig.mqttDataOff;
    
    gpio_isr_handler_add((uint32_t)intrParams->intrPin, iot_gpio_isr_intr_handler, intrParams);
    xTaskCreate(iot_gpio_isr_task_queue_handler, intrConfig.intrTaskName, 2048, intrParams, 1, NULL);
    
    return ESP_OK;
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

    iot_mqtt_message_t mqttMessage = {
        .topic = intrParams->mqttSubTopic,
        .qos = 0,
        .retain = true,

    };


    if(intrPin) {
        mqttMessage.data = intrParams->mqttDataOn;
//        iot_send_mqtt(intrParams->mqttSubTopic, intrParams->mqttDataOn);
        iot_send_mqtt(&mqttMessage);
    } else {
        mqttMessage.data = intrParams->mqttDataOff;
//        iot_send_mqtt(intrParams->mqttSubTopic, intrParams->mqttDataOff);
        iot_send_mqtt(&mqttMessage);
    }

    if(indicator) { gpio_set_level(intrParams->outPin, intrPin); }

}


static void iot_intr_switch_one_shot_and_timer(iot_isr_params_t *intrParams)
{
    
    // Check to see if there is an outpit GPIO (state
    // indicator) assigned in the interrupt config.
    bool indicator = intrParams->outPin != GPIO_NUM_NC ? true : false;

    // Check to see if the input in inverted. Inverted input are
    // used when the sensor uses an "Acrive Low" output.
    iot_mqtt_message_t mqttMessage = {
        .topic = intrParams->mqttSubTopic,
        .qos = 0,
        .retain = true,
        .data = intrParams->mqttDataOn,
    };

    iot_send_mqtt(&mqttMessage);

    if(indicator) { 
        gpio_set_level(intrParams->outPin, true);
        timerHandle = xTimerCreate("One Shot Timer", pdMS_TO_TICKS(intrParams->timerDelay), false, intrParams, &gpio_timer_intr_callback);
        xTimerStartFromISR(timerHandle, (int*)10);
        
    }
}


static void gpio_timer_intr_callback(TimerHandle_t timer) 
{   
    iot_isr_params_t* intrParams = (iot_isr_params_t*)pvTimerGetTimerID(timer);
    if(intrParams->outPin != GPIO_NUM_NC) {
        gpio_set_level(intrParams->outPin, false);
    }
    
    if(intrParams->intrSwitchType == IOT_ISR_SWITCH_TIMER) {
        iot_mqtt_message_t mqttMessage = {
            .topic = intrParams->mqttSubTopic,
            .qos = 0,
            .retain = true,
            .data = intrParams->mqttDataOff,
        };  
        iot_send_mqtt(&mqttMessage);
    }
}
