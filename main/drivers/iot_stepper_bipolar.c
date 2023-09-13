#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt_tx.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "iot_stepper_common.h"
#include "esp_timer.h"
#include <string.h>
#include "cJSON.h"
#include "../iot_globals.h"
#include "../iot_structs.h"
#include "../iot_enums.h"
#include "../iot_defines.h"
#include "../iot_utils.h"
#include "../iot_mqtt.h"
#include "driver/gpio_filter.h"

static char* set_stepper_resolution(int resolution, iot_stepper_config_t* configEntry);
static void set_stepper_resolution_pins(iot_stepper_config_t* configEntry, int m0, int m1, int m2);
static void run_stepper(rmt_channel_handle_t motorChan, iot_stepper_config_t* callbackData, iot_stepper_move_command_t command);
void stepper_mqtt_subscribe_handler(void* data, esp_mqtt_event_t* event);
static void  limit_isr_intr_handler(void* arg);
static void linit_isr_task_queue_handler(void *params);
static esp_err_t iot_stepper_limit_setup(iot_stepper_config_t* configEntry, rmt_channel_handle_t* motorChan);
bool iot_mqtt_configure(iot_stepper_config_t* configEntry);
static void stepper_limit_glitch_setup(gpio_num_t limitGpio);


static char* set_stepper_resolution(int resolution, iot_stepper_config_t* configEntry) {
    switch (resolution) {
        case 1:
            switch (configEntry->stepperDriver) {
                case IOT_STEPPER_A4988:
                case IOT_STEPPER_DRV8825:
                    set_stepper_resolution_pins(configEntry, 0, 0, 0);
                    break;
                
                default:
                    break;
            }
            break;
        case 2:
            switch (configEntry->stepperDriver) {
                case IOT_STEPPER_A4988:
                case IOT_STEPPER_DRV8825:
                    set_stepper_resolution_pins(configEntry, 1, 0, 0);
                    break;
                
                default:
                    break;
            }
            break;
        case 4:
            switch (configEntry->stepperDriver) {
                case IOT_STEPPER_A4988:
                case IOT_STEPPER_DRV8825:
                    set_stepper_resolution_pins(configEntry, 0, 1, 0);
                    break;
                
                default:
                    break;
            }
            break;
        case 8:
            switch (configEntry->stepperDriver) {
                case IOT_STEPPER_A4988:
                case IOT_STEPPER_DRV8825:
                    set_stepper_resolution_pins(configEntry, 1, 1, 0);
                    break;
                
                default:
                    break;
            }
            break;
        case 16:
            switch (configEntry->stepperDriver) {
                case IOT_STEPPER_A4988:
                    set_stepper_resolution_pins(configEntry, 1, 1, 1);
                    break;
                case IOT_STEPPER_DRV8825:
                    set_stepper_resolution_pins(configEntry, 0, 0, 1);
                    break;
                
                default:
                    break;
            }
            break;
        case 32:
            switch (configEntry->stepperDriver) {
                case IOT_STEPPER_DRV8825:
                    set_stepper_resolution_pins(configEntry, 1, 0, 1);
                    break;
                
                default:
                    // Need to flsh this out.
                    return "ERROR";
                    break;
            }
        
        default:
            // Silently ignore invalid resolutions.
            break;
    }
    return NULL;
}


static void set_stepper_resolution_pins(iot_stepper_config_t* configEntry, int m0, int m1, int m2) {
    gpio_set_level(configEntry->pinMicroStep0, m0);
    gpio_set_level(configEntry->pinMicroStep1, m1);
    gpio_set_level(configEntry->pinMicroStep2, m2);
}


bool iot_mqtt_configure(iot_stepper_config_t* configEntry) {
    bool ret = true;
    rmt_channel_handle_t* motorChan = malloc(sizeof(rmt_channel_handle_t));

    ESP_LOGI(TAG, "Initialize EN, DIR & MicroStepping GPIOs");
    gpio_config_t en_dir_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = BIT64(configEntry->pinEnable) |
                        BIT64(configEntry->pinDirection) |
                        BIT64(configEntry->pinMicroStep0) |
                        BIT64(configEntry->pinMicroStep1) |
                        BIT64(configEntry->pinMicroStep2),
    };
    ESP_ERROR_CHECK(gpio_config(&en_dir_gpio_config));

    ESP_LOGI(TAG, "Create RMT TX channel");
    //rmt_channel_handle_t motorChan = NULL;
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // select clock source
        .gpio_num = configEntry->pinStep,
        .mem_block_symbols = 64,
        .resolution_hz = STEP_MOTOR_RESOLUTION_HZ,
        .trans_queue_depth = 1, // set the number of transactions that can be pending in the background
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, motorChan));

    // Place pointer for RMT motor transmit channel handle
    // in iot_gpio_array using index of pinStep.
    iot_gpio_array[configEntry->pinStep] = (int)motorChan;

    iot_mqtt_subscribe_callback_t* mqttCallback = calloc(1, sizeof(iot_mqtt_subscribe_callback_t));
    mqttCallback->callbackData = configEntry;
    mqttCallback->callbackFunc = &stepper_mqtt_subscribe_handler;

    char* subscribeTopic = concat(baseTopic, configEntry->mqttConfig->mqttSubscribe);    
    ESP_LOGI(TAG,"Subscribing to MQTT Topic: %s with QoS of %d", subscribeTopic, configEntry->mqttConfig->mqttSubscribeQos - 1);
    iot_mqtt_callback_add(subscribeTopic, mqttCallback);
    esp_mqtt_client_subscribe(iotMqttClient,subscribeTopic,configEntry->mqttConfig->mqttSubscribeQos - 1);

    iot_stepper_limit_setup(configEntry, motorChan);

    return ret;
}


void stepper_mqtt_subscribe_handler(void* data, esp_mqtt_event_t* event) {
    iot_stepper_config_t* callbackData = (iot_stepper_config_t*)data;
    printf("%s\n", callbackData->stepperTaskName);
    rmt_channel_handle_t* motorChan = (rmt_channel_handle_t*)iot_gpio_array[callbackData->pinStep];

    cJSON* object = cJSON_Parse(event->data);
    if(cJSON_HasObjectItem(object, "stepperCommands")) {
        char* move = "move";
        char* wait = "wait";
        if(cJSON_HasObjectItem(object, "stepperCommands")){

            cJSON* stepperCommandsArray = cJSON_GetObjectItem(object, "stepperCommands");
            if(cJSON_IsArray(stepperCommandsArray)) {
                for (int i = 0 ; i < cJSON_GetArraySize(stepperCommandsArray) ; i++) {
                    cJSON* stepperCommand = cJSON_GetArrayItem(stepperCommandsArray, i);
                    if(cJSON_HasObjectItem(stepperCommand, "type")) {
                        char* type = cJSON_GetObjectItem(stepperCommand, "type")->valuestring;
                        if(strcmp(type, move) == 0) {
                            iot_stepper_move_command_t command = {
                                .stepsPerSecond = cJSON_GetObjectItem(stepperCommand, "stepsPerSecond")->valueint,
                                .steps = cJSON_GetObjectItem(stepperCommand, "steps")->valueint,
                                .clockwise = cJSON_IsTrue(cJSON_GetObjectItem(stepperCommand, "clockwise")) == true ? true : false,
                                .microstepSetting = cJSON_GetObjectItem(stepperCommand, "microstepSetting")->valueint
                            };
                            bool notLimitCW = !gpio_get_level(callbackData->limitCW);
                            bool notLimitCCW = !gpio_get_level(callbackData->limitCCW);

                            set_stepper_resolution(command.microstepSetting, callbackData);
                            if((notLimitCCW & !command.clockwise)) {
                                rmt_enable(*motorChan);
                                run_stepper(*motorChan, callbackData, command);
                                rmt_disable(*motorChan);
                            }
                            if((notLimitCW & command.clockwise)) {
                                rmt_enable(*motorChan);
                                run_stepper(*motorChan, callbackData, command);
                                rmt_disable(*motorChan);
                            }

                        } else if (strcmp(type, wait) == 0) {
                            int delay = cJSON_GetObjectItem(stepperCommand, "duration")->valueint;
                            vTaskDelay(delay / portTICK_PERIOD_MS);
                        }
                    }
                }
            } else {
                ESP_LOGI(TAG, "stepperCommand JSON array not found.");
            }

        } else {
            ESP_LOGI(TAG, "stepperCommand JSON object not found.");
        }
    }
}


static void run_stepper(rmt_channel_handle_t motorChan, iot_stepper_config_t* callbackData, iot_stepper_move_command_t command) {
    rmt_transmit_config_t uniform_tx_config = {
        .loop_count = command.steps,
    };

    bool normalizedDirection = false;
    if(callbackData->reverse) {
        normalizedDirection = command.clockwise;
    } else {
        normalizedDirection = !command.clockwise;
    }
    
    gpio_set_level(callbackData->pinDirection, normalizedDirection);
    ESP_ERROR_CHECK(rmt_transmit(motorChan, uniformMotorEncoder, &command.stepsPerSecond, sizeof(command.stepsPerSecond), &uniform_tx_config));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(motorChan, -1));
}


static esp_err_t iot_stepper_limit_setup(iot_stepper_config_t* configEntry, rmt_channel_handle_t* motorChan) {
    gpio_config_t limitPinConfig = {
        .pin_bit_mask = BIT64(configEntry->limitCW) | BIT64(configEntry->limitCCW),
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = true,
        .pull_up_en = false,
        .intr_type = GPIO_INTR_POSEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&limitPinConfig));
    stepper_limit_glitch_setup(configEntry->limitCW);
    stepper_limit_glitch_setup(configEntry->limitCCW);


    gpio_isr_handler_add((uint32_t)configEntry->limitCW, limit_isr_intr_handler, motorChan);
    xTaskCreate(linit_isr_task_queue_handler, concat(configEntry->stepperTaskName, "-CW"), 2048, motorChan, 35, NULL);
    ESP_LOGD(TAG, "Interrupt Queue Created");

    gpio_isr_handler_add((uint32_t)configEntry->limitCCW, limit_isr_intr_handler, motorChan);
    xTaskCreate(linit_isr_task_queue_handler, concat(configEntry->stepperTaskName, "-CCW"), 2048, motorChan, 35, NULL);
    ESP_LOGD(TAG, "Interrupt Queue Created");
    
    return ESP_OK;
}


static void  limit_isr_intr_handler(void* arg) {
  	rmt_channel_handle_t motorChan = (rmt_channel_handle_t) arg;
	xQueueSendToBackFromISR(stepperLimitIntrQueue, motorChan, NULL);
}


static void linit_isr_task_queue_handler(void *params) // xTaskCreate
{
    while(true) {
        rmt_channel_handle_t* motorChan = malloc(sizeof(rmt_channel_handle_t));
        ESP_LOGI(TAG, "Waiting on MQTT interrupt queue");
		xQueueReceive(stepperLimitIntrQueue, motorChan, portMAX_DELAY);
        printf("Limit Reached.\n");
        rmt_disable(*motorChan);
        free(motorChan);
    }
}


static void stepper_limit_glitch_setup(gpio_num_t limitGpio) {
    // Configure GLitch Filter on interrupt pin.
    gpio_pin_glitch_filter_config_t* glitchFilterConfig = malloc(sizeof(gpio_pin_glitch_filter_config_t));
    glitchFilterConfig->clk_src = 4; // SOC_MOD_CLK_PLL_F80M
    glitchFilterConfig->gpio_num = limitGpio;
    gpio_glitch_filter_handle_t* glitchFilter = malloc(sizeof(gpio_glitch_filter_handle_t));
    ESP_ERROR_CHECK(gpio_new_pin_glitch_filter(glitchFilterConfig, glitchFilter));
    ESP_ERROR_CHECK(gpio_glitch_filter_enable(*glitchFilter));
}
