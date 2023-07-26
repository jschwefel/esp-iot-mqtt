#include "iot_globals.h"
#include "iot_value_defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/gpio_filter.h"
#include "iot_interrupt.h"

#include "hal/gpio_ll.h"
#include "esp_sleep.h"
#include "esp_log.h"
//#include "log.h"
//#include "mqtt.h"



static void iot_gpio_intr_task_handler(void *params);
static void iot_gpio_isr_handler(void* arg);
static void iot_gpio_intr_task_handler(void *params);


static void iot_gpio_isr_handler(void* arg)
{
    //ESP_LOGI(TAG, "GPIO ISR triggered");
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(interruptQueue, &gpio_num, NULL);
}

static void iot_gpio_control_task(void *params)
{
    iot_isr_params_t* intrParams = (iot_isr_params_t*) params;
    while (true)
    {
        ESP_LOGI(TAG, "Control Task' -- \tParam: %i", (int)params);
        // Call the intr task handler function, passing all params
        intrParams->intrFunc(params);
    }    
}



static void iot_gpio_intr_task_handler(void *params)
{
    iot_isr_params_t* intrParams = (iot_isr_params_t*) params;
    if (xQueueReceive(interruptQueue, &intrParams->intrPin, portMAX_DELAY)) {
        ESP_LOGI(TAG, "Params passed from 'iot_gpio_intr_pin' -- intrPin: %i\toutPin: %i\tParam: %i", intrParams->intrPin, intrParams->outPin, (int)intrParams);
        uint32_t level = intrParams->outInvert ? !gpio_get_level(intrParams->intrPin) : gpio_get_level(intrParams->intrPin);
        
        gpio_set_level(intrParams->outPin, gpio_get_level(intrParams->intrPin));
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

    if (1) {
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
    intrParams->intrFunc = &iot_gpio_intr_task_handler;
    intrParams->intrPin = intrConfig.intrPin;
    intrParams->outPin = intrConfig.outPin;
    intrParams->outInvert = intrConfig.intrISR.outInvert;
    intrParams->mqttSubTopic = intrConfig.intrISR.mqttSubTopic;
    intrParams->mqttDataHigh = intrConfig.intrISR.mqttDataHigh;
    intrParams->mqttDataLow = intrConfig.intrISR.mqttDataLow;

    ESP_LOGE(TAG, "Params passed from 'iot_gpio_control_task' -- Pin: %i\tFunc: %i\t%i", intrParams->intrPin, (int)intrParams->intrFunc, (int)intrParams);

    BaseType_t xReturned = xTaskCreate(iot_gpio_control_task, intrConfig.intrTaskName, 2048, (void*)intrParams, 1, NULL);
    if(xReturned == pdPASS) {
        ESP_LOGI(TAG, "Interrupt Task in FreeRTOS created.");
    } else {
        ESP_LOGE(TAG, "Interrupt Task in FreeRTOS creation FAILED.");
    }
    
    ESP_ERROR_CHECK(gpio_isr_handler_add(intrConfig.intrPin, iot_gpio_isr_handler, (void *)intrParams->intrPin));

    return ESP_OK;


}

