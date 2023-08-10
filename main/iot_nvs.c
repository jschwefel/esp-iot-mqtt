#include <stdio.h>
#include <string.h>
#include "iot_globals.h"
#include "iot_nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"


nvs_handle_t iot_init_flash(nvs_handle_t handle, char* nvs_namespace) 
{
    esp_err_t ret = nvs_flash_init();
    // If the flash cannot initialize, or there is a return other than
    // 'ESP_OK', it is a fatal error
    ESP_ERROR_CHECK(ret); 

    // I know this works, but reading the docs, the '!= ESP_OK'
    // is redundant
    // ESP_ERROR_CHECK(nvs_open(namespace, NVS_READWRITE, &handle) != ESP_OK);
    ESP_ERROR_CHECK(nvs_open(nvs_namespace, NVS_READWRITE, &handle));
    
    // vns_open() uses 'handle' as OUT param. 
    return handle;
};

char* iot_nvs_get_str_value(const char* key)
{
    // If either of the two calls fail, abort.
    size_t value_size;
    ESP_ERROR_CHECK(nvs_get_str(iot_nvs_user_handle, key, NULL, &value_size));
    char* value = malloc(value_size);
    ESP_ERROR_CHECK(nvs_get_str(iot_nvs_user_handle, key, value, &value_size));
    return value;
}

bool iot_nvs_set_str_value(const char* key, char* value)
{
    ESP_ERROR_CHECK(nvs_set_str(iot_nvs_user_handle, key, value));
    return true;
}

bool iot_nvs_set_int_value(const char* key, int32_t value)
{

    ESP_ERROR_CHECK(nvs_set_i32(iot_nvs_user_handle, key, value));
    return true;
}

int32_t iot_nvs_get_int_value(const char* key)
{
    int32_t value = -1;
    ESP_ERROR_CHECK(nvs_get_i32(iot_nvs_user_handle, key, &value));
    return value;
}

bool iot_nvs_set_blobstr_value(const char* key, char* value)
{
    //printf("%s\n", key);
    //printf("%s\n", value);
    //printf("%d\n", strlen(value));
    ESP_ERROR_CHECK(nvs_set_blob(iot_nvs_user_handle, key, value, strlen(value)));

    return true;
}

char* iot_nvs_get_blobstr_value(const char* key)
{
    // If either of the two calls fail, abort.
    size_t value_size;
    ESP_ERROR_CHECK(nvs_get_blob(iot_nvs_user_handle, key, NULL, &value_size));
    char* value = malloc(value_size);
    ESP_ERROR_CHECK(nvs_get_blob(iot_nvs_user_handle, key, value, &value_size));
    return value;
}
