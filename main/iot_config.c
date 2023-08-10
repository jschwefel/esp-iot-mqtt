#include <stdio.h>
#include <stdbool.h>
#include "iot_globals.h"
#include "iot_enums.h"
#include "iot_structs.h"
#include "iot_nvs.h"
#include "iot_defines.h"
#include "iot_utils.h"
#include "cJSON.h"

static void serializeConfig(cJSON* tempConfigJson, iot_config_item_t* entry, cJSON* confArray, void* serializeFunction);
static iot_config_linked_list_t* deserializeConfig(cJSON* tempConfigJson, void* deserializeFunction);
static cJSON* serialize_iot_intr_switch_simple_config(void* configItemPtr);
static iot_config_linked_list_t* deserialize_iot_intr_switch_simple_config(char* key, cJSON* configJson);

static iot_config_linked_list_t* deserialize_dummy(char* key, cJSON* configJson);

cJSON* iot_intr_switch_simple_config_array = NULL;
cJSON* iot_intr_dummy_config_array = NULL;

void iot_save_config(iot_config_linked_list_t* iotConfigHead) {
    iot_config_linked_list_t* prev = calloc(1, sizeof(iot_config_linked_list_t));                
    iot_config_linked_list_t* config = iotConfigHead;
    while(true) {
        iot_config_item_t* entry = config->configEntry;
        cJSON* tempConfigJson = cJSON_CreateObject();
        
        switch(entry->configItemType) {
            case IOT_CONFIG_SIMPLE_SWITCH:
                if(iot_intr_switch_simple_config_array == NULL) {
                    iot_intr_switch_simple_config_array = cJSON_CreateArray();
                }
                serializeConfig(tempConfigJson, entry, iot_intr_switch_simple_config_array, &serialize_iot_intr_switch_simple_config);
            break;

            case IOT_CONFIG_DUMMY_TEST:
                if(iot_intr_dummy_config_array == NULL) {
                    iot_intr_dummy_config_array = cJSON_CreateArray();
                }
                serializeConfig(tempConfigJson, entry, iot_intr_dummy_config_array, &serialize_iot_intr_switch_simple_config);
            break;
        };

        prev = config;
        if(config->next == NULL) {
            break;
        }

        void* next = config->next;
        config = (iot_config_linked_list_t*)next;
    }

    free(prev);

    if(iot_intr_switch_simple_config_array != NULL) {
        char key[ENOUGH];
        sprintf(key, "%d", IOT_CONFIG_SIMPLE_SWITCH);
        cJSON_AddItemToObject(iotConfiguration, key, iot_intr_switch_simple_config_array);
    }
    
    if(iot_intr_dummy_config_array != NULL) {
        char key[ENOUGH];
        sprintf(key, "%d", IOT_CONFIG_DUMMY_TEST);
        cJSON_AddItemToObject(iotConfiguration, key, iot_intr_dummy_config_array);
    }

    char* outJson = cJSON_Print(iotConfiguration);
    iot_nvs_set_blobstr_value(IOT_CONFIG_KEY, outJson);
    printf(outJson);
}

// I am sure this can be optimized like deserializeConfig
iot_config_linked_list_t* iot_open_config(void) {
    char* configBlob = iot_nvs_get_blobstr_value(IOT_CONFIG_KEY);
    cJSON* configJson = cJSON_Parse(configBlob);
    cJSON* node = configJson->child;
    //iot_config_linked_list_t* iotConfigHead  = calloc(1, sizeof(iot_config_linked_list_t));
    iot_config_linked_list_t* configHead = calloc(1, sizeof(iot_config_linked_list_t));
    //iot_config_linked_list_t* config = calloc(1, sizeof(iot_config_linked_list_t));
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

static void serializeConfig(cJSON* tempConfigJson, iot_config_item_t* entry, cJSON* confArray, void* serializeFunction) {
    cJSON* (*serializer)(void*) = serializeFunction;
    cJSON_AddItemToObject(tempConfigJson, entry->configKey, (*serializer)(entry->configItem));
    cJSON_AddItemToArray(confArray, tempConfigJson);
}
// https://stackoverflow.com/a/58499138
static iot_config_linked_list_t* deserializeConfig(cJSON* arrayNode, void* deserializeFunction) {
    iot_config_linked_list_t* (*deserializer)(char*, cJSON*) = deserializeFunction;
    int arraySize = cJSON_GetArraySize(arrayNode); 
    iot_config_linked_list_t* head = NULL;
    iot_config_linked_list_t* tail = NULL;
    for(int i = 0; i < arraySize; i++) {
        iot_config_linked_list_t* temp = calloc(1, sizeof(*temp));
        cJSON* arrayItem = cJSON_GetArrayItem(arrayNode, i)->child;
        temp = deserializer(arrayItem->string, arrayItem);
        temp->next = NULL;
        if (tail == NULL) {
            head = temp;
            tail = temp;
        } else {
            tail->next = temp;
            tail = temp;
        }
    }
    return head;
}


static cJSON* serialize_iot_intr_switch_simple_config(void* configItemPtr) {
    iot_intr_switch_simple_config_t* configItem = (iot_intr_switch_simple_config_t*)configItemPtr;
    cJSON* configJson = cJSON_CreateObject();
    cJSON_AddStringToObject(configJson, "intrTaskName", configItem->intrTaskName);
    cJSON_AddStringToObject(configJson, "mqttSubTopic", configItem->mqttSubTopic);
    cJSON_AddStringToObject(configJson, "mqttDataOn", configItem->mqttDataOn);
    cJSON_AddStringToObject(configJson, "mqttDataOff", configItem->mqttDataOff);
    cJSON_AddNumberToObject(configJson, "intrPin", configItem->intrPin);
    cJSON_AddNumberToObject(configJson, "intrPull", configItem->intrPull);
    cJSON_AddNumberToObject(configJson, "intrType", configItem->intrType);
    cJSON_AddNumberToObject(configJson, "intrSimpleSwitchType", configItem->intrSimpleSwitchType);
    cJSON_AddNumberToObject(configJson, "outPin", configItem->outPin);
    cJSON_AddNumberToObject(configJson, "outPull", configItem->outPull);
    cJSON_AddNumberToObject(configJson, "timerDelay", configItem->timerDelay);
    cJSON_AddNumberToObject(configJson, "outInvert", configItem->outInvert ? true : false);
 
    return configJson;
}

static iot_config_linked_list_t* deserialize_iot_intr_switch_simple_config(char* key, cJSON* configJson) {
    iot_intr_switch_simple_config_t* simpleSwitchConfig = malloc(sizeof(iot_intr_switch_simple_config_t));
    if(cJSON_HasObjectItem(configJson, "intrTaskName")) {simpleSwitchConfig->intrTaskName = cJSON_GetObjectItem(configJson, "intrTaskName")->valuestring;}
    if(cJSON_HasObjectItem(configJson, "mqttSubTopic")) {simpleSwitchConfig->mqttSubTopic = cJSON_GetObjectItem(configJson, "mqttSubTopic")->valuestring;}
    if(cJSON_HasObjectItem(configJson, "mqttDataOn")) {simpleSwitchConfig->mqttDataOn = cJSON_GetObjectItem(configJson, "mqttDataOn")->valuestring;}
    if(cJSON_HasObjectItem(configJson, "mqttDataOff")) {simpleSwitchConfig->mqttDataOff = cJSON_GetObjectItem(configJson, "mqttDataOff")->valuestring;}
    if(cJSON_HasObjectItem(configJson, "intrPin")) {simpleSwitchConfig->intrPin = cJSON_GetObjectItem(configJson, "intrPin")->valuedouble;}
    if(cJSON_HasObjectItem(configJson, "intrPull")) {simpleSwitchConfig->intrPull = cJSON_GetObjectItem(configJson, "intrPull")->valuedouble;}
    if(cJSON_HasObjectItem(configJson, "intrType")) {simpleSwitchConfig->intrType = cJSON_GetObjectItem(configJson, "intrType")->valuedouble;}
    if(cJSON_HasObjectItem(configJson, "intrSimpleSwitchType")) {simpleSwitchConfig->intrSimpleSwitchType = cJSON_GetObjectItem(configJson, "intrSimpleSwitchType")->valuedouble;}
    if(cJSON_HasObjectItem(configJson, "outPin")) {simpleSwitchConfig->outPin = cJSON_GetObjectItem(configJson, "outPin")->valuedouble;}
    if(cJSON_HasObjectItem(configJson, "outPull")) {simpleSwitchConfig->outPull = cJSON_GetObjectItem(configJson, "outPull")->valuedouble;}
    if(cJSON_HasObjectItem(configJson, "timerDelay")) {simpleSwitchConfig->timerDelay = cJSON_GetObjectItem(configJson, "timerDelay")->valuedouble;}
    if(cJSON_HasObjectItem(configJson, "outInvert")) {simpleSwitchConfig->outInvert = cJSON_GetObjectItem(configJson, "outInvert")->valuedouble != 0 ? true : false;}

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


static iot_config_linked_list_t* deserialize_dummy(char* key, cJSON* configJson) {
    iot_intr_switch_simple_config_t* simpleSwitchConfig = malloc(sizeof(iot_intr_switch_simple_config_t));
    if(cJSON_HasObjectItem(configJson, "intrTaskName")) {simpleSwitchConfig->intrTaskName = cJSON_GetObjectItem(configJson, "intrTaskName")->valuestring;}
    if(cJSON_HasObjectItem(configJson, "mqttSubTopic")) {simpleSwitchConfig->mqttSubTopic = cJSON_GetObjectItem(configJson, "mqttSubTopic")->valuestring;}
    if(cJSON_HasObjectItem(configJson, "mqttDataOn")) {simpleSwitchConfig->mqttDataOn = cJSON_GetObjectItem(configJson, "mqttDataOn")->valuestring;}
    if(cJSON_HasObjectItem(configJson, "mqttDataOff")) {simpleSwitchConfig->mqttDataOff = cJSON_GetObjectItem(configJson, "mqttDataOff")->valuestring;}
    if(cJSON_HasObjectItem(configJson, "intrPin")) {simpleSwitchConfig->intrPin = cJSON_GetObjectItem(configJson, "intrPin")->valuedouble;}
    if(cJSON_HasObjectItem(configJson, "intrPull")) {simpleSwitchConfig->intrPull = cJSON_GetObjectItem(configJson, "intrPull")->valuedouble;}
    if(cJSON_HasObjectItem(configJson, "intrType")) {simpleSwitchConfig->intrType = cJSON_GetObjectItem(configJson, "intrType")->valuedouble;}
    if(cJSON_HasObjectItem(configJson, "intrSimpleSwitchType")) {simpleSwitchConfig->intrSimpleSwitchType = cJSON_GetObjectItem(configJson, "intrSimpleSwitchType")->valuedouble;}
    if(cJSON_HasObjectItem(configJson, "outPin")) {simpleSwitchConfig->outPin = cJSON_GetObjectItem(configJson, "outPin")->valuedouble;}
    if(cJSON_HasObjectItem(configJson, "outPull")) {simpleSwitchConfig->outPull = cJSON_GetObjectItem(configJson, "outPull")->valuedouble;}
    if(cJSON_HasObjectItem(configJson, "timerDelay")) {simpleSwitchConfig->timerDelay = cJSON_GetObjectItem(configJson, "timerDelay")->valuedouble;}
    if(cJSON_HasObjectItem(configJson, "outInvert")) {simpleSwitchConfig->outInvert = cJSON_GetObjectItem(configJson, "outInvert")->valuedouble;}

    iot_config_linked_list_t* configListItem = calloc(1, sizeof(iot_config_linked_list_t*));
    iot_config_item_t* configItem = calloc(1, sizeof(iot_config_item_t));
    configItem->configItemType = IOT_CONFIG_DUMMY_TEST;
    configItem->configItem = simpleSwitchConfig;
    configItem->configKey = key;

    // calloc should make all bytes NULL, but GDB shows a value for ->next.
    // Nulling it out.
    configListItem->next = NULL;
    configListItem->configEntry = configItem;
    return configListItem;
}