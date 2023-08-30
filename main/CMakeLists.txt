#idf_component_register(SRCS "iot_mqtt.c" "iot_utils.c" "iot_nvs.c" "esp-iot-mqtt.c"
#                            "iot_config.c"
#                            "iot_globals.c"
#                            "iot_interrupt.c"
#                            "iot_wifi.c"
#                            "iot_httpd.c"
#                            "../URLDecode/urldecode.c"
#
#
#                            "dummy.c"

idf_component_register(SRC_DIRS "." 
                                "drivers"
                                "../URLDecode/"
                         
                    INCLUDE_DIRS "."
                                "../URLDecode/"
                                "drivers")

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error=char-subscripts -Wno-error=unknown-pragmas)                                

add_custom_command( OUTPUT ${PROJECT_BINARY_DIR}/web/index.html
    COMMAND ./pytools/spiffs-minify.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    VERBATIM)

add_custom_target(run ALL
    DEPENDS ${PROJECT_BINARY_DIR}/web/index.html)

spiffs_create_partition_image(storage ../build/web FLASH_IN_PROJECT DEPENDS ${PROJECT_BINARY_DIR}/web/index.html)
