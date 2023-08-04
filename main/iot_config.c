#include <stdio.h>
#include <stdlib.h>
#include <cJSON.h>
#include "iot_globals.h"
#include "iot_globals.h"
#include "iot_value_defines.h"
#include "esp_log.h"
#include "benhoyt_hashtable.h"

static iot_config_linked_list_t* iot_list_config_entries(iot_config_item_type_t configItemType);

void iot_create_config_hashtable(void)
{
    iotConfigHashTable = ht_create();
}

const char* iot_insert_simple_switch_intr_config_entry(iot_intr_config_t* value)
{

    iot_config_item_t* configItem = malloc(sizeof(iot_config_item_t));
    configItem->configItemType = IOT_CONFIG_SIMPLE_SWITCH;
    configItem->configKey = value->intrTaskName;
    configItem->configItem = value;

    return ht_set(iotConfigHashTable, configItem->configKey, configItem);
}

iot_config_item_t* iot_retrieve_simple_switch_intr_config_entry(char* key)
{
    iot_config_item_t* configEntry = ht_get(iotConfigHashTable, key);
    if(configEntry->configItemType == IOT_CONFIG_SIMPLE_SWITCH) {
        return configEntry->configItem;
    }
    return NULL;
}

iot_config_linked_list_t* iot_list_simple_switch_intr_config_entries() 
{
    return iot_list_config_entries(IOT_CONFIG_SIMPLE_SWITCH);
    
}

static iot_config_linked_list_t* iot_list_config_entries(iot_config_item_type_t configItemType)
{
    hti iterator = ht_iterator(iotConfigHashTable);
    iot_config_linked_list_t* HEAD = calloc(1, sizeof(iot_config_linked_list_t));
    iot_config_linked_list_t* prev = NULL;

    if(ht_next(&iterator)) {
        HEAD->configEntry = iterator.value;
        prev = HEAD;
    }


    while(ht_next(&iterator)) {
        iot_config_linked_list_t* curr = calloc(1, sizeof(iot_config_linked_list_t));
        curr->configEntry = iterator.value;
        prev->next = curr;
        prev = curr;
    }
    
    return HEAD;
}