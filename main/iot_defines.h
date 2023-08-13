#pragma once
#include <limits.h>

#define IOT_CONFIG_KEY          "config_data"


#define IOT_DEFAULT_WIFI_BASE               "esp-iot-mqtt-"
#define IOT_WIFI_MODE_AP                    1
#define IOT_WIFI_MODE_STA                   2
#define IOT_ISR_ONE_SHOT_OUT_DELAY          500
#define IOT_CONFIG_HASHTABLE_SIZE           100


/********************************************************************
 * Definitions of all NVS Keys
********************************************************************/

#define IOT_KEY_WIFI_MODE                   "wifi-mode"
#define IOT_KEY_WIFI_SSID                   "wifi-ssid"
#define IOT_KEY_WIFI_PASS                   "wifi-passwd"




#define ENOUGH ((CHAR_BIT * sizeof(int) - 1) / 3 + 2)

#define INDEX_HTML_PATH "/spiffs/index.html"