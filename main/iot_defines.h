#pragma once
#include <limits.h>

#define ENOUGH                                      ((CHAR_BIT * sizeof(int) - 1) / 3 + 2)


#define IOT_CONFIG_KEY                              "config_data"


#define IOT_DEFAULT_WIFI_BASE                       "esp-iot-mqtt-"
#define IOT_WIFI_MODE_AP                            1
#define IOT_WIFI_MODE_STA                           2
#define IOT_ISR_ONE_SHOT_OUT_DELAY                  500
#define IOT_CONFIG_HASHTABLE_SIZE                   100


/************************************************************************************************
 * Definitions of all NVS Keys
************************************************************************************************/

#define IOT_KEY_WIFI_MODE                           "wifi-mode"
#define IOT_KEY_WIFI_SSID                           "wifi-ssid"
#define IOT_KEY_WIFI_PASS                           "wifi-passwd"

#define IOT_KEY_MQTT_BROKER                         "mqtt-broker"





#define IOT_HTTPD_RESPONSE_BUFFER_ADD                100
#define IOT_WEB_SYSTEM_SETTING_URI                  "/system-settings.html"
#define IOT_WEB_IOT_SETTING_URI                     "/iot-settings.html"

#define IOT_CONFIG_TYPE_MAX_ENTRIES                 9       // 0 based

#define IOT_CONFIG_TYPE_PREFIX                      "iot-%02d-"
#define IOT_CONFIG_MAX_CONFIG_STR_LEN               255

#define IOT_ISR_ONE_SHOT_OUT_PIN_DEFAULT_DELAY      250
