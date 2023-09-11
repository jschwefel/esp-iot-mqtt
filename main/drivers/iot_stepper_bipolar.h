#pragma once


bool iot_mqtt_configure(iot_stepper_config_t* config);
void stepper_mqtt_subscribe_handler(void* data, esp_mqtt_event_t* event);