#pragma once
#include "iot_globals.h"


void iot_start_mqtt(void);
//void iot_send_mqtt(char* service, char* payload);
void iot_send_mqtt(iot_mqtt_message_t* mqttMessage);