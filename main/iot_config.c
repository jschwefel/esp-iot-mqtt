#include <stdio.h>
#include <stdbool.h>
#include "iot_globals.h"
#include "iot_enums.h"
#include "iot_structs.h"
#include "iot_nvs.h"
#include "iot_defines.h"
#include "iot_utils.h"
#include "iot_httpd.h"
#include "cJSON.h"
#include "../URLDecode/urldecode.h"
#include "esp_log.h"
#include "iot_simple_switch.h"
#include "iot_config.h"






//cJSON* iot_intr_switch_simple_config_array = NULL;
//cJSON* iot_intr_dummy_config_array = NULL;


void iot_save_config(iot_config_linked_list_t* iotConfigHead) {
    iot_config_linked_list_t* prev = calloc(1, sizeof(iot_config_linked_list_t));                
    iot_config_linked_list_t* config = iotConfigHead;

    cJSON* iot_intr_switch_simple_config_array = NULL;
    cJSON* iot_intr_dummy_config_array = NULL;

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
    cJSON* iotConfiguration = cJSON_CreateObject();

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
    free(iotConfiguration);
}


void serializeConfig(cJSON* tempConfigJson, iot_config_item_t* entry, cJSON* confArray, void* serializeFunction) {
    cJSON* (*serializer)(void*) = serializeFunction;
    cJSON_AddItemToObject(tempConfigJson, entry->configKey, (*serializer)(entry->configItem));
    cJSON_AddItemToArray(confArray, tempConfigJson);
}


// https://stackoverflow.com/a/58499138
iot_config_linked_list_t* deserializeConfig(cJSON* arrayNode, void* deserializeFunction) {
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


char* iot_system_settings_populate_html(char* rawHtml) {
    char* wifiSsid = iot_nvs_get_str_value(IOT_KEY_WIFI_SSID);
    char* mqttBroker = iot_nvs_get_str_value(IOT_KEY_MQTT_BROKER);

    size_t htmlLen =  strlen(rawHtml)
                    + strlen(wifiSsid)
                    + strlen(mqttBroker);

    char* procHtml = calloc(1, htmlLen);
    sprintf(procHtml, rawHtml, wifiSsid, mqttBroker);
    free(rawHtml);
    return procHtml;
}


void iot_iot_settings_process_config_update(char* queryString) {
    char configItemTypeChar[5];         // Max of 9999 config item types.
    iot_config_linked_list_t* head = NULL;
    iot_config_linked_list_t* tail = NULL;

    for(int i = 0; i <= IOT_CONFIG_TYPE_MAX_ENTRIES; i++) {
        char* fstring = IOT_CONFIG_TYPE_PREFIX;
        char* configItemEntry = malloc(sizeof(char) * 12); // 'iot-XX-type' = 11 char + NULL
        sprintf(configItemEntry, fstring, i);
        configItemEntry = concat(configItemEntry, (char*)"type");
        esp_err_t err = httpd_query_key_value(queryString, configItemEntry, configItemTypeChar, 4);
        if(err != ESP_OK) {
            // Should flesh this out more.
            break;
        }
        iot_config_item_type_t configItemType = atoi(configItemTypeChar);
        printf("%02d\n", configItemType);
        iot_config_linked_list_t* temp = calloc(1, sizeof(*temp));
        switch (configItemType) {
            case IOT_CONFIG_SIMPLE_SWITCH:
                ESP_LOGI(TAG, "Query string: %s\n", queryString);
                temp = iot_iot_settings_process_config_update_simple_switch(i, queryString);
                break;
            
            default:
                break;
        }
        
        temp->next = NULL;
        if (tail == NULL) {
            head = temp;
            tail = temp;
        } else {
            tail->next = temp;
            tail = temp;
        }

        //free(configItemEntry);
    }
    iot_save_config(head);
}
