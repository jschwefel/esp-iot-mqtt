#include "iot_globals.h"
#include "iot_value_defines.h"
#include <stdio.h>
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

#define LED_PIN GPIO_NUM_1

static void iot_gpio_isr_handler(void* arg);
static void iot_gpio_control_task(void *params);
static gpio_config_t* iot_set_gpio_intr_pin_config(gpio_num_t gpioPin, gpio_mode_t mode, bool pullUp, bool pullDown, gpio_int_type_t intrType);


static void iot_gpio_isr_handler(void* arg)
{
    //ESP_LOGI(TAG, "GPIO ISR triggered");
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(interruptQueue, &gpio_num, NULL);
}




static void iot_gpio_control_task(void *params)
{
    iot_intr_params_t* intrParams = (iot_intr_params_t*) params;
    while (true)
    {
        intrParams->intrFunc(params);
        ESP_LOGE(TAG, "Params passed from 'iot_gpio_control_task' -- Pin: %i\tFunc: %i", intrParams->intrPin, (int)intrParams->intrFunc );
        //vTaskDelay(50);
    }    
}

esp_err_t iot_intr_pin_config(gpio_num_t gpioPin, gpio_mode_t mode, bool pullUp, bool pullDown, gpio_int_type_t intrType)
{
    gpio_config_t* pinConfig = iot_set_gpio_intr_pin_config(gpioPin, mode, pullUp, pullDown, intrType);
    ESP_ERROR_CHECK(gpio_config(pinConfig));
    free(pinConfig);
    gpio_pin_glitch_filter_config_t* glitchFilterConfig = malloc(sizeof(gpio_pin_glitch_filter_config_t));
    glitchFilterConfig->clk_src = 4; // SOC_MOD_CLK_PLL_F80M
    glitchFilterConfig->gpio_num = gpioPin;
    gpio_glitch_filter_handle_t* glitchFilter = malloc(sizeof(gpio_glitch_filter_handle_t));
    ESP_ERROR_CHECK(gpio_new_pin_glitch_filter(glitchFilterConfig, glitchFilter));
    ESP_ERROR_CHECK(gpio_glitch_filter_enable(*glitchFilter));

    free(glitchFilterConfig);
    return ESP_OK;
}


esp_err_t iot_io_pin_config(gpio_num_t gpioPin, gpio_mode_t mode, bool pullUp, bool pullDown)
{
    gpio_config_t* pinConfig = iot_set_gpio_intr_pin_config(gpioPin, mode, pullUp, pullDown, GPIO_INTR_DISABLE);
    esp_err_t ret = gpio_config(pinConfig);
    free(pinConfig);
    return ret;
}

static gpio_config_t* iot_set_gpio_intr_pin_config(gpio_num_t gpioPin, gpio_mode_t mode, bool pullUp, bool pullDown, gpio_int_type_t intrType)
{
    gpio_config_t* pinConfig = malloc(sizeof(gpio_config_t));
    pinConfig->pin_bit_mask = BIT64(gpioPin);
    pinConfig->mode = mode;
    pinConfig->pull_down_en = pullDown ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
    pinConfig->pull_up_en = pullUp ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    pinConfig->intr_type = intrType;

    return pinConfig;

}

void iot_config_gpio_interrupt(gpio_num_t intrPin, void* intrISR)
{


/////////////////////////////////////////////////////////////////////////////

    //ESP_ERROR_CHECK(gpio_set_intr_type(gpio_int_num, GPIO_INTR_ANYEDGE));


     iot_intr_params_t* intrParams = malloc(sizeof(iot_intr_params_t));
     intrParams->intrPin = intrPin;
     intrParams->intrFunc = intrISR;

    ESP_LOGE(TAG, "Params passed from 'iot_gpio_control_task' -- Pin: %i\tFunc: %i", intrParams->intrPin, (int)intrParams->intrFunc );

    BaseType_t xReturned = xTaskCreate(iot_gpio_control_task, "IOT_GPIO_Control_Task", 2048, intrParams, 1, NULL);
    if(xReturned == pdPASS)
    {
        printf("LED Control Task created.\n");
    }
    else 
    {
        printf("LED Control Task FAILED.\n");
    }
    
    ESP_ERROR_CHECK(gpio_isr_handler_add(intrPin, iot_gpio_isr_handler, (void *)intrParams));

    

////////////////////////////////////////////////////////////////////////////


}


