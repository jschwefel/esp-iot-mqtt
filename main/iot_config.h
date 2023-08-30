#pragma once

void iot_save_config(iot_config_linked_list_t* iotConfigHead);

char* iot_system_settings_populate_html(char* rawHtml);
void iot_iot_settings_process_config_update(char* queryString);
void serializeConfig(cJSON* tempConfigJson, iot_config_item_t* entry, cJSON* confArray, void* serializeFunction);
iot_config_linked_list_t* deserializeConfig(cJSON* tempConfigJson, void* deserializeFunction);