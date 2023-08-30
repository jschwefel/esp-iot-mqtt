#pragma once

void iot_save_config(iot_config_linked_list_t* iotConfigHead);
iot_config_linked_list_t* iot_open_config(void);
char* iot_system_settings_populate_html(char* rawHtml);
void iot_iot_settings_process_config_update(char* queryString);