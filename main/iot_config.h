#pragma once
#include "iot_globals.h"

void iot_create_config_hashtable(void);
const char* iot_insert_simple_switch_intr_config_entry(iot_intr_config_t* value);
iot_config_item_t* iot_retrieve_config_entry(char* key);
iot_config_linked_list_t* iot_list_simple_switch_intr_config_entries();
