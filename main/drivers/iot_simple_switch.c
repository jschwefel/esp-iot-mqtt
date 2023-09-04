#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/portmacro.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "driver/gpio.h"
#include "driver/gpio_filter.h"
#include "cJSON.h"
#include "iot_simple_switch.h"
#include "../iot_globals.h"
#include "../iot_enums.h"
#include "../iot_mqtt.h"
#include "../iot_utils.h"
#include "../iot_config.h"
#include "../iot_enums.h"
#include "../iot_config.h"
#include "../iot_nvs.h"
#include "../iot_defines.h"
#include "../URLDecode/urldecode.h"


TimerHandle_t timerHandle;

static void iot_gpio_isr_task_queue_handler(void *params);
static void iot_gpio_isr_intr_handler(void* arg);
static void iot_intr_switch_toggle(iot_intr_switch_simple_config_t *intrParams);
static void iot_intr_switch_one_shot_and_timer(iot_intr_switch_simple_config_t *intrParams);
static void gpio_timer_intr_callback(TimerHandle_t timer);
static esp_err_t iot_intr_simple_switch_setup(iot_intr_switch_simple_config_t* intrConfig);
static iot_mqtt_message_t* set_mqtt_message(char* subTopic, char* payload);

void iot_conf_controller(iot_config_linked_list_t* configList) {
    while(true) {
        iot_mqtt_subscribe_callback_t* mqttCallback = calloc(1, sizeof(iot_mqtt_subscribe_callback_t));
        iot_config_item_t* configItem = configList->configEntry;

        iot_intr_switch_simple_config_t* X = configItem->configItem;
        iot_mqtt_config_t* Y = X->mqttConfig;
        ESP_LOGD(TAG, "%d", Y->mqttTopicQos );
        switch(configItem->configItemType) {
            case IOT_CONFIG_SIMPLE_SWITCH :

                iot_intr_switch_simple_config_t* configEntry = (iot_intr_switch_simple_config_t*)(configItem->configItem);
                iot_intr_simple_switch_setup(configEntry);
                mqttCallback->callbackData = configEntry;
                mqttCallback->callbackFunc = &simple_switch_mqtt_subscribe_handler;
                char* subscribeTopic = NULL;
                if (configEntry->mqttConfig->mqttSubscribe != NULL) {
                    subscribeTopic = concat(baseTopic, configEntry->mqttConfig->mqttSubscribe);    
                    ESP_LOGI(TAG,"Subscribing to MQTT Topic: %s", subscribeTopic);
                    iot_mqtt_callback_add(subscribeTopic, mqttCallback);
                    esp_mqtt_client_subscribe(iotMqttClient,subscribeTopic,0);
                    esp_mqtt_client_subscribe(iotMqttClient,subscribeTopic,1);                
                }             
                break;

            case IOT_CONFIG_DUMMY_TEST :
                break;
                //iot_intr_switch_simple_config_t* configEntry = (iot_intr_switch_simple_config_t*)(configItem->configItem);
                //iot_intr_simple_switch_setup(configEntry); 
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
        //intrParams->mqttConfig = calloc(1, sizeof(iot_mqtt_config_t));
        ESP_LOGI(TAG, "Waiting on interrupt queue");
		BaseType_t rc = xQueueReceive(simpleSwitchInreQueue, intrParams, portMAX_DELAY);
        ESP_LOGW(TAG, "%d", intrParams->intrSimpleSwitchType);
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
//            .mode = GPIO_MODE_OUTPUT,
            .mode = GPIO_MODE_INPUT_OUTPUT,
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

    gpio_isr_handler_add((uint32_t)intrConfig->intrPin, iot_gpio_isr_intr_handler, intrConfig);
    xTaskCreate(iot_gpio_isr_task_queue_handler, intrConfig->intrTaskName, 2048, intrConfig, 1, NULL);
    ESP_LOGD(TAG, "Interrupt Queue Created");
    
    return ESP_OK;
}

static iot_mqtt_message_t* set_mqtt_message(char* subTopic, char* payload) {
    iot_mqtt_message_t* mqttMessage = calloc(1, sizeof(iot_mqtt_message_t));
    
    mqttMessage->topic = subTopic;
    mqttMessage->qos = 1;
    mqttMessage->retain = false;
    if(payload != NULL) {
        mqttMessage->data = payload;
    }

    return mqttMessage;

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
 
    iot_mqtt_message_t* mqttMessage = set_mqtt_message(intrParams->mqttConfig->mqttTopic, NULL);

    if(intrPinNorm != 0) {
        mqttMessage->data = intrParams->mqttDataOn;
        iot_send_mqtt(mqttMessage);
    } else {
        mqttMessage->data = intrParams->mqttDataOff;
        iot_send_mqtt(mqttMessage);
    }
    free(mqttMessage);

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
    iot_mqtt_message_t* mqttMessage = set_mqtt_message(intrParams->mqttConfig->mqttTopic, intrParams->mqttDataOn);
    iot_send_mqtt(mqttMessage);
    free(mqttMessage);

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
        iot_mqtt_message_t* mqttMessage = set_mqtt_message(intrParams->mqttConfig->mqttTopic, intrParams->mqttDataOff);
        iot_send_mqtt(mqttMessage);
        free(mqttMessage);
    }
}

// I am sure this can be optimized like deserializeConfig
iot_config_linked_list_t* iot_open_config(void) {
    char* configBlob = iot_nvs_get_blobstr_value(IOT_CONFIG_KEY);
    cJSON* configJson = cJSON_Parse(configBlob);
    cJSON* node = configJson->child;
    iot_config_linked_list_t* configHead = calloc(1, sizeof(iot_config_linked_list_t));
    iot_config_linked_list_t* config = NULL;
    iot_config_linked_list_t** configPtrPtr = &configHead;
    iot_config_linked_list_t* tempConfig = NULL;
    while(true) {
        switch((iot_config_item_type_t)atoi(node->string)) {
            case IOT_CONFIG_SIMPLE_SWITCH :
                config = deserializeConfig(node, &deserialize_iot_intr_switch_simple_config);
                tempConfig = getLastEntry(configHead);
                tempConfig->next = config;
            break;

            case IOT_CONFIG_DUMMY_TEST :
                break;
                config = deserializeConfig(node,  &deserialize_dummy);
                tempConfig = getLastEntry(configHead);
                tempConfig->next = config;
            break;
        }

        if(node->next == NULL) {

            break;
        }
        node = node->next;
    }
    iot_config_linked_list_t* configPtr = *configPtrPtr;
    if(configPtr->configEntry == NULL) {
        return configPtr->next;
    }
    return configPtr;
}

cJSON* serialize_iot_intr_switch_simple_config(void* configItemPtr) {
    iot_intr_switch_simple_config_t* configItem = (iot_intr_switch_simple_config_t*)configItemPtr;
    cJSON* configJson = cJSON_CreateObject();
    cJSON_AddStringToObject(configJson, "intrTaskName", configItem->intrTaskName);
    //Need to fix this up to make it a subobject in the JSON
    cJSON_AddStringToObject(configJson, "mqttTopic", configItem->mqttConfig->mqttTopic);
    cJSON_AddStringToObject(configJson, "mqttSubscribe", configItem->mqttConfig->mqttSubscribe);
    cJSON_AddNumberToObject(configJson, "mqttTopicQos", configItem->mqttConfig->mqttTopicQos);
    cJSON_AddNumberToObject(configJson, "mqttSubscribeQos", configItem->mqttConfig->mqttSubscribeQos);
    cJSON_AddStringToObject(configJson, "mqttDataOn", configItem->mqttDataOn);
    cJSON_AddStringToObject(configJson, "mqttDataOff", configItem->mqttDataOff);
    cJSON_AddNumberToObject(configJson, "intrPin", configItem->intrPin);
    cJSON_AddNumberToObject(configJson, "intrPull", configItem->intrPull);
    cJSON_AddNumberToObject(configJson, "intrType", configItem->intrType);
    cJSON_AddNumberToObject(configJson, "intrSimpleSwitchType", configItem->intrSimpleSwitchType);
    cJSON_AddNumberToObject(configJson, "outPin", configItem->outPin);
    cJSON_AddNumberToObject(configJson, "outPull", configItem->outPull);
    cJSON_AddNumberToObject(configJson, "timerDelay", configItem->timerDelay);
    cJSON_AddNumberToObject(configJson, "inputInvert", configItem->inputInvert ? true : false);

    //printf("%s\n\n\n", cJSON_Print(configJson));

    return configJson;
}

iot_config_linked_list_t* deserialize_iot_intr_switch_simple_config(char* key, cJSON* configJson) {
    printf("%s\nBob\n\n", cJSON_Print(configJson));
    iot_intr_switch_simple_config_t* simpleSwitchConfig = malloc(sizeof(iot_intr_switch_simple_config_t));
    iot_mqtt_config_t* mqttConfig = calloc(1, sizeof(iot_mqtt_config_t));
    if(cJSON_HasObjectItem(configJson, "intrTaskName")) {simpleSwitchConfig->intrTaskName = cJSON_GetObjectItem(configJson, "intrTaskName")->valuestring;}
    if(cJSON_HasObjectItem(configJson, "mqttTopic")) {
        mqttConfig->mqttTopic = cJSON_GetObjectItem(configJson, "mqttTopic")->valuestring;

        }
    if(cJSON_HasObjectItem(configJson, "mqttSubScribe")) {mqttConfig->mqttSubscribe = cJSON_GetObjectItem(configJson, "mqttSubscribe")->valuestring;}
    if(cJSON_HasObjectItem(configJson, "mqttTopicQos")) {mqttConfig->mqttTopicQos = cJSON_GetObjectItem(configJson, "mqttTopicQos")->valueint;}
    if(cJSON_HasObjectItem(configJson, "mqttSubScribeQos")) {mqttConfig->mqttSubscribeQos = cJSON_GetObjectItem(configJson, "mqttSubscribeQos")->valueint;}    
    if(cJSON_HasObjectItem(configJson, "mqttDataOn")) {simpleSwitchConfig->mqttDataOn = cJSON_GetObjectItem(configJson, "mqttDataOn")->valuestring;}
    if(cJSON_HasObjectItem(configJson, "mqttDataOff")) {simpleSwitchConfig->mqttDataOff = cJSON_GetObjectItem(configJson, "mqttDataOff")->valuestring;}
    if(cJSON_HasObjectItem(configJson, "intrPin")) {simpleSwitchConfig->intrPin = cJSON_GetObjectItem(configJson, "intrPin")->valueint;}
    if(cJSON_HasObjectItem(configJson, "intrPull")) {simpleSwitchConfig->intrPull = cJSON_GetObjectItem(configJson, "intrPull")->valueint;}
    if(cJSON_HasObjectItem(configJson, "intrType")) {simpleSwitchConfig->intrType = cJSON_GetObjectItem(configJson, "intrType")->valueint;}
    if(cJSON_HasObjectItem(configJson, "intrSimpleSwitchType")) {simpleSwitchConfig->intrSimpleSwitchType = cJSON_GetObjectItem(configJson, "intrSimpleSwitchType")->valueint;}
    if(cJSON_HasObjectItem(configJson, "outPin")) {simpleSwitchConfig->outPin = cJSON_GetObjectItem(configJson, "outPin")->valueint;}
    if(cJSON_HasObjectItem(configJson, "outPull")) {simpleSwitchConfig->outPull = cJSON_GetObjectItem(configJson, "outPull")->valueint;}
    if(cJSON_HasObjectItem(configJson, "timerDelay")) {simpleSwitchConfig->timerDelay = cJSON_GetObjectItem(configJson, "timerDelay")->valueint;}
    if(cJSON_HasObjectItem(configJson, "inputInvert")) {simpleSwitchConfig->inputInvert = cJSON_GetObjectItem(configJson, "inputInvert")->valueint != 0 ? true : false;}

    simpleSwitchConfig->mqttConfig = mqttConfig;

    iot_config_linked_list_t* configListItem = calloc(1, sizeof(iot_config_linked_list_t*));
    iot_config_item_t* configItem = calloc(1, sizeof(iot_config_item_t));
    configItem->configItemType = IOT_CONFIG_SIMPLE_SWITCH;
    configItem->configItem = simpleSwitchConfig;
    configItem->configKey = key;

    // calloc should make all bytes NULL, but GDB shows a value for ->next.
    // Nulling it out.
    configListItem->next = NULL;
    configListItem->configEntry = configItem;
    return configListItem;
}

iot_config_linked_list_t* iot_iot_settings_process_config_update_simple_switch(int sequence, char* queryString) {
    ESP_LOGI(TAG, "Query string: %s\n", queryString);
    char* prefix = malloc(sizeof(char) * 8);     // 'iot-XX-' = 7 char + NULL
    sprintf(prefix, IOT_CONFIG_TYPE_PREFIX, sequence);
    
    // Technically this would be a memory leak, but the end of this process is a restart, so it is moot.
    iot_intr_switch_simple_config_t* config = malloc(sizeof(iot_intr_switch_simple_config_t));
    config->mqttConfig = calloc(1, sizeof(iot_mqtt_config_t));
    char tempChar[IOT_CONFIG_MAX_CONFIG_STR_LEN];
    iot_config_item_t* configItem = NULL;

    // Make sure we have a task name
    if (httpd_query_key_value(queryString, concat(prefix, (char*)"intrTaskName"), tempChar, IOT_CONFIG_MAX_CONFIG_STR_LEN) == ESP_OK) {
        config->intrTaskName = urlDecode(tempChar);
        
        httpd_query_key_value(queryString, concat(prefix, (char*)"intrPin"), tempChar, IOT_CONFIG_MAX_CONFIG_STR_LEN);
        config->intrPin = atoi(tempChar);

        httpd_query_key_value(queryString, concat(prefix, (char*)"inputInvert"), tempChar, IOT_CONFIG_MAX_CONFIG_STR_LEN);
        config->inputInvert= atoi(tempChar);

        httpd_query_key_value(queryString, concat(prefix, (char*)"intrPull"), tempChar, IOT_CONFIG_MAX_CONFIG_STR_LEN);
        config->intrPull = atoi(tempChar);

        httpd_query_key_value(queryString, concat(prefix, (char*)"intrType"), tempChar, IOT_CONFIG_MAX_CONFIG_STR_LEN);
        config->intrType = atoi(tempChar);

        httpd_query_key_value(queryString, concat(prefix, (char*)"intrSimpleSwitchType"), tempChar, IOT_CONFIG_MAX_CONFIG_STR_LEN);
        config->intrSimpleSwitchType = atoi(tempChar);

        if(httpd_query_key_value(queryString, concat(prefix, (char*)"mqttTopic"), tempChar, IOT_CONFIG_MAX_CONFIG_STR_LEN) == ESP_OK) {
            config->mqttConfig->mqttTopic = urlDecode(tempChar);
            httpd_query_key_value(queryString, concat(prefix, (char*)"mqttTopicQos"), tempChar, IOT_CONFIG_MAX_CONFIG_STR_LEN);
            config->mqttConfig->mqttTopicQos = atoi(tempChar);
        }
            
        if(httpd_query_key_value(queryString, concat(prefix, (char*)"mqttSubscribe"), tempChar, IOT_CONFIG_MAX_CONFIG_STR_LEN) == ESP_OK) {
            config->mqttConfig->mqttSubscribe = urlDecode(tempChar);
            httpd_query_key_value(queryString, concat(prefix, (char*)"mqttSubscribeQos"), tempChar, IOT_CONFIG_MAX_CONFIG_STR_LEN);
            config->mqttConfig->mqttSubscribeQos = atoi(tempChar);
        }
            

        httpd_query_key_value(queryString, concat(prefix, (char*)"mqttDataOn"), tempChar, IOT_CONFIG_MAX_CONFIG_STR_LEN);
        config->mqttDataOn = urlDecode(tempChar);

        httpd_query_key_value(queryString, concat(prefix, (char*)"mqttDataOff"), tempChar, IOT_CONFIG_MAX_CONFIG_STR_LEN);
        config->mqttDataOff = urlDecode(tempChar);

        if(httpd_query_key_value(queryString, concat(prefix, (char*)"outPin"), tempChar, IOT_CONFIG_MAX_CONFIG_STR_LEN) == ESP_OK) {
            int test = atoi(tempChar);
            if(test != GPIO_NUM_NC) {
                config->outPin= test;
                httpd_query_key_value(queryString, concat(prefix, (char*)"outPull"), tempChar, IOT_CONFIG_MAX_CONFIG_STR_LEN);
                config->outPull = atoi(tempChar);
            }
        }        

        if((config->intrSimpleSwitchType == IOT_INTR_SWITCH_TIMER_POS) || config->intrSimpleSwitchType == IOT_INTR_SWITCH_TIMER_NEG) {
            httpd_query_key_value(queryString, concat(prefix, (char*)"timerDelay"), tempChar, IOT_CONFIG_MAX_CONFIG_STR_LEN);
            config->timerDelay = atoi(tempChar);  
        } else {
            config->timerDelay = 500;  
        }
        configItem = malloc(sizeof(iot_config_item_t));
        configItem->configItemType = IOT_CONFIG_SIMPLE_SWITCH;
        configItem->configKey = config->intrTaskName;
        configItem->configItem = config;

    }
    iot_config_linked_list_t* configList = calloc(1, sizeof(iot_config_linked_list_t*));
    configList->configEntry = configItem;
    configList->next = NULL;

    free(prefix);

    return configList;

}

void simple_switch_mqtt_subscribe_handler(void* data) {
    iot_intr_switch_simple_config_t* callbackData = (iot_intr_switch_simple_config_t*)data;
    int output = gpio_get_level(callbackData->outPin);
    printf("Outpin: %d\tState: %d\n", callbackData->outPin, output);
    if(output) {
        iot_mqtt_message_t* mqttMessage = set_mqtt_message(callbackData->mqttConfig->mqttTopic, callbackData->mqttDataOn);
        iot_send_mqtt(mqttMessage);
        free(mqttMessage);
    } else {
        iot_mqtt_message_t* mqttMessage = set_mqtt_message(callbackData->mqttConfig->mqttTopic, callbackData->mqttDataOff);
        iot_send_mqtt(mqttMessage);
        free(mqttMessage);

    }
}