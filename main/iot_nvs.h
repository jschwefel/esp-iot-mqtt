/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#pragma once


int32_t iot_nvs_load_int_value_if_exist(const char *key);
bool iot_nvs_set_int_value(const char *key,int32_t value);
bool iot_nvs_set_str_value(const char *key,char *value);
char *iot_nvs_load_str_value(const char *key);
nvs_handle_t iot_init_flash(nvs_handle_t handle,char *nvs_namespace);
