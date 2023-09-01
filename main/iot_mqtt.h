#pragma once
#include "iot_globals.h"
#include "iot_structs.h"


void iot_start_mqtt(void);
//void iot_send_mqtt(char* service, char* payload);
void iot_send_mqtt(iot_mqtt_message_t* mqttMessage);
void iot_mqtt_callback_add(char* key, iot_mqtt_subscribe_callback_t* callback);
iot_mqtt_subscribe_callback_t* iot_mqtt_subscribe_get(char* key);