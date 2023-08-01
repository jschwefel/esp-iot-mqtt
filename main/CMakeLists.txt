idf_component_register(SRCS "benhoyt_hashtable.c" "iot_mqtt.c" "main.c"
                            "iot_globals.c"
                            "iot_nvs.c"
                            "iot_wifi.c"
                            "iot_utils.c"
                            "iot_gpio_interrupt.c"
                            "iot_mqtt.c"
                            "iot_config.c"
                            "benhoyt_hashtable.c"
                    INCLUDE_DIRS "")
#spiffs_create_partition_image(storage ../data FLASH_IN_PROJECT)
