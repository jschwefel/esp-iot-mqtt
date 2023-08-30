#include "iot_globals.h"
#include "iot_enums.h"
#include "iot_mqtt.h"
#include "iot_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "iot_interrupt.h"
#include "freertos/portmacro.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/gpio_filter.h"
#include "iot_config.h"
#include "iot_enums.h"



TimerHandle_t timerHandle;

static void iot_gpio_isr_task_queue_handler(void *params);
static void iot_gpio_isr_intr_handler(void* arg);
static void iot_intr_switch_toggle(iot_intr_switch_simple_config_t *intrParams);
static void iot_intr_switch_one_shot_and_timer(iot_intr_switch_simple_config_t *intrParams);
static void gpio_timer_intr_callback(TimerHandle_t timer);
static esp_err_t iot_intr_simple_switch_setup(iot_intr_switch_simple_config_t* intrConfig);

void iot_conf_controller(iot_config_linked_list_t* configList) {
    while(true) {
        iot_config_item_t* configItem = configList->configEntry;
        switch(configItem->configItemType) {
            case IOT_CONFIG_SIMPLE_SWITCH :
                iot_intr_simple_switch_setup((iot_intr_switch_simple_config_t*)(configItem->configItem));   
                break;

            case IOT_CONFIG_DUMMY_TEST :
                break;
                iot_intr_simple_switch_setup((iot_intr_switch_simple_config_t*)(configItem->configItem));   
                break;
        }
        if(configList->next == NULL) {
            break;
        }
        configList = configList->next;
    }
}

static void  iot_gpio_isr_intr_handler(void* arg) // gpio_isr_handler_add
{
  	iot_intr_switch_simple_config_t* intrParams = (iot_intr_switch_simple_config_t*) arg;
    gpio_intr_disable(intrParams->intrPin);
	xQueueSendToBackFromISR(simpleSwitchInreQueue, intrParams, NULL);
    gpio_intr_enable(intrParams->intrPin);
}

static void iot_gpio_isr_task_queue_handler(void *params) // xTaskCreate
{
    while(true) {
        iot_intr_switch_simple_config_t* intrParams = malloc(sizeof(iot_intr_switch_simple_config_t));
        ESP_LOGI(TAG, "Waiting on interrupt queue");
		BaseType_t rc = xQueueReceive(simpleSwitchInreQueue, intrParams, portMAX_DELAY);

        switch(intrParams->intrSimpleSwitchType) {
            case IOT_INTR_SWITCH_ONE_SHOT_POS :
            case IOT_INTR_SWITCH_ONE_SHOT_NEG :
            case IOT_INTR_SWITCH_TIMER_POS :
            case IOT_INTR_SWITCH_TIMER_NEG :
                iot_intr_switch_one_shot_and_timer(intrParams);
                break;
            
            case IOT_INTR_SWITCH_TOGGLE :
                iot_intr_switch_toggle(intrParams);
                break;
        }
		ESP_LOGI(TAG, "Woke from interrupt queue wait: %d on GPIO: %i", rc, intrParams->intrPin);
    }
}

static esp_err_t iot_intr_simple_switch_setup(iot_intr_switch_simple_config_t* intrConfig)
{
    // Confiure pins.
    // Pretty sure there is a better way. But for now, it works.
    bool intrPullUp = ((intrConfig->intrPull == IOT_GPIO_PULL_UP) || (intrConfig->intrPull == IOT_GPIO_PULL_BOTH)) ? true : false;
    bool intrPullDown = ((intrConfig->intrPull == IOT_GPIO_PULL_DOWN) || (intrConfig->intrPull == IOT_GPIO_PULL_BOTH)) ? true : false;
    bool outPullUp = ((intrConfig->outPull == IOT_GPIO_PULL_UP) || (intrConfig->intrPull == IOT_GPIO_PULL_BOTH)) ? true : false;
    bool outPullDown = ((intrConfig->outPull == IOT_GPIO_PULL_DOWN) || (intrConfig->intrPull == IOT_GPIO_PULL_BOTH)) ? true : false;

/* 
    bool intrPullUp = ((intrConfig->intrPull == 0) || (intrConfig->intrPull == 2)) ? true : false;
    bool intrPullDown = ((intrConfig->intrPull == 1) || (intrConfig->intrPull == 2)) ? true : false;
    bool outPullUp = ((intrConfig->outPull == 0) || (intrConfig->intrPull == 2)) ? true : false;
    bool outPullDown = ((intrConfig->outPull == 1) || (intrConfig->intrPull == 2)) ? true : false;

 */
    gpio_int_type_t intr = GPIO_INTR_DISABLE;
    switch(intrConfig->intrSimpleSwitchType) {
        case IOT_INTR_SWITCH_TOGGLE :
            intr = GPIO_INTR_ANYEDGE;
        break;
    
        case IOT_INTR_SWITCH_ONE_SHOT_POS :
        case IOT_INTR_SWITCH_TIMER_POS :
            intr = GPIO_INTR_NEGEDGE;
        break;

        case IOT_INTR_SWITCH_ONE_SHOT_NEG :
        case IOT_INTR_SWITCH_TIMER_NEG :
            intr = GPIO_INTR_POSEDGE;
        break;
    }

    gpio_config_t intrPinConfig = {
        .pin_bit_mask = BIT64(intrConfig->intrPin),
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = intrPullDown,
        .pull_up_en = intrPullUp,
        .intr_type = intr,


    };
    ESP_ERROR_CHECK(gpio_config(&intrPinConfig));

    if (intrConfig->outPin != GPIO_NUM_NC) {
        gpio_config_t outPinConfig = {
            .pin_bit_mask = BIT64(intrConfig->outPin),
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
    glitchFilterConfig->gpio_num = intrConfig->intrPin;
    gpio_glitch_filter_handle_t* glitchFilter = malloc(sizeof(gpio_glitch_filter_handle_t));
    ESP_ERROR_CHECK(gpio_new_pin_glitch_filter(glitchFilterConfig, glitchFilter));
    ESP_ERROR_CHECK(gpio_glitch_filter_enable(*glitchFilter));

    ESP_LOGI(TAG, "This is it");
    gpio_isr_handler_add((uint32_t)intrConfig->intrPin, iot_gpio_isr_intr_handler, intrConfig);
    xTaskCreate(iot_gpio_isr_task_queue_handler, intrConfig->intrTaskName, 2048, intrConfig, 1, NULL);
    
    return ESP_OK;
}


static void iot_intr_switch_toggle(iot_intr_switch_simple_config_t *intrParams)
{
    // The toggle switch type is used for On/Off type switches.
    // This type of switch will trigger on 'GPIO_INTR_ANYEDGE'.
    // Typical uses are door and window open/close switches.
    
    // Check to see if there is an outpit GPIO (state
    // indicator) assigned in the interrupt config.
    bool indicator = intrParams->outPin != GPIO_NUM_NC ? true : false;

    // Check to see if the input in inverted. Inverted input are
    // used when the sensor uses an "Acrive Low" output.
    uint32_t intrPinNorm = gpio_normailized_state(intrParams->inputInvert, intrParams->intrPin);
    iot_mqtt_message_t mqttMessage = {
        .topic = intrParams->mqttSubTopic,
        .qos = 0,
        .retain = true,
    };


    if(intrPinNorm != 0) {
        mqttMessage.data = intrParams->mqttDataOn;
        iot_send_mqtt(&mqttMessage);
    } else {
        mqttMessage.data = intrParams->mqttDataOff;
        iot_send_mqtt(&mqttMessage);
    }
    //gpio_set_level(1,1);

    if(indicator) { 
        gpio_set_level(intrParams->outPin, intrPinNorm); 
    }

}


static void iot_intr_switch_one_shot_and_timer(iot_intr_switch_simple_config_t *intrParams)
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
    iot_intr_switch_simple_config_t* intrParams = (iot_intr_switch_simple_config_t*)pvTimerGetTimerID(timer);
    if(intrParams->outPin != GPIO_NUM_NC) {
        gpio_set_level(intrParams->outPin, false);
    }
    
    if((intrParams->intrSimpleSwitchType == IOT_INTR_SWITCH_TIMER_POS) || (intrParams->intrSimpleSwitchType == IOT_INTR_SWITCH_TIMER_NEG)) {
        iot_mqtt_message_t mqttMessage = {
            .topic = intrParams->mqttSubTopic,
            .qos = 0,
            .retain = true,
            .data = intrParams->mqttDataOff,
        };  
        iot_send_mqtt(&mqttMessage);
    }
}


void iot_intr_gpio_set_config(char* intrName, gpio_num_t intrPin, iot_gpio_pull_t intrPull,
    gpio_int_type_t intrType, iot_simple_switch_type_t intrSimpleSwitchType, int timeDelay,
    gpio_num_t outPin, iot_gpio_pull_t outPull, bool inputInvert, char* mqttSubTopic,
    char* mqttOn, char* mqttOff)
{
    iot_intr_switch_simple_config_t* intrGpioConfig = malloc(sizeof(iot_intr_switch_simple_config_t));
        intrGpioConfig->intrTaskName = intrName;
        intrGpioConfig->intrPin = intrPin;
        intrGpioConfig->intrPull = intrPull;
        intrGpioConfig->intrType = intrType;
        intrGpioConfig->intrSimpleSwitchType = intrSimpleSwitchType;
        intrGpioConfig->timerDelay = timeDelay;
        intrGpioConfig->outPin = outPin;
        intrGpioConfig->outPull = outPull;
        intrGpioConfig->inputInvert = inputInvert;
        intrGpioConfig->mqttSubTopic = mqttSubTopic;
        intrGpioConfig->mqttDataOn = mqttOn;
        intrGpioConfig->mqttDataOff = mqttOff;
}