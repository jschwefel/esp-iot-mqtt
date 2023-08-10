#include <stdio.h>
#include <stdbool.h>
#include "iot_enums.h"
#include "iot_structs.h"
#include "iot_enums.h"
#include "iot_globals.h"
#include "iot_config.h"
#include "driver/gpio.h"


iot_config_linked_list_t* dummy_intiConfig(void) {

    iot_intr_switch_simple_config_t* jasonTest = calloc(1, sizeof(iot_intr_switch_simple_config_t));
        jasonTest->intrTaskName = "Task Inter 1";
        jasonTest->intrPin = GPIO_NUM_0;
        jasonTest->intrSimpleSwitchType = IOT_INTR_SWITCH_TOGGLE;
        jasonTest->outPin = GPIO_NUM_1;
        jasonTest->outInvert = true;
        jasonTest->timerDelay = 5000;
        jasonTest->mqttSubTopic = "/JSONTest";

    iot_config_item_t* configItemTest0 = calloc(1, sizeof(iot_config_item_t));
        configItemTest0->configItem = jasonTest;
        configItemTest0->configKey = "JSON_Test-1";
        configItemTest0->configItemType = IOT_CONFIG_SIMPLE_SWITCH;

    iot_config_item_t* configItemTest1 = calloc(1, sizeof(iot_config_item_t));
        configItemTest1->configItem = jasonTest;
        configItemTest1->configKey = "Config_Test-1";
        configItemTest1->configItemType = IOT_CONFIG_DUMMY_TEST;


    iot_intr_switch_simple_config_t* lindaTest = calloc(1, sizeof(iot_intr_switch_simple_config_t));
        lindaTest->intrTaskName = "Task Inter 2";
        lindaTest->intrPin = GPIO_NUM_2;
        lindaTest->intrSimpleSwitchType = IOT_INTR_SWITCH_ONE_SHOT_POS;
        lindaTest->intrPull = IOT_GPIO_PULL_DOWN;
        lindaTest->intrType = GPIO_INTR_POSEDGE;
        lindaTest->outPin = GPIO_NUM_3;
        lindaTest->outInvert = false;
        lindaTest->outPull = IOT_GPIO_PULL_DOWN;
        lindaTest->timerDelay = 500;
        lindaTest->mqttSubTopic = "/NopeNope";
        lindaTest->mqttDataOn = "On";
        lindaTest->mqttDataOff = "Off";
    



    iot_config_item_t* configItemTest3 = calloc(1, sizeof(iot_config_item_t));
        configItemTest3->configItem = lindaTest;
        configItemTest3->configKey = "JSON_Test-2";
        configItemTest3->configItemType = IOT_CONFIG_SIMPLE_SWITCH;
    


    iot_config_item_t* configItemTest4 = calloc(1, sizeof(iot_config_item_t));
        configItemTest4->configItem = lindaTest;
        configItemTest4->configKey = "Config_Test-2";
        configItemTest4->configItemType = IOT_CONFIG_DUMMY_TEST;
    


    iot_config_linked_list_t* c4 = calloc(1, sizeof(iot_config_linked_list_t));
    c4->configEntry = configItemTest4;
    iot_config_linked_list_t* c3 = calloc(1, sizeof(iot_config_linked_list_t));
    c3->next = c4;
    c3->configEntry = configItemTest3;
    iot_config_linked_list_t* c1 = calloc(1, sizeof(iot_config_linked_list_t));
    c1->next = c3;
    c1->configEntry = configItemTest1;
    iot_config_linked_list_t* iotConfigHead = calloc(1, sizeof(iot_config_linked_list_t));
    iotConfigHead->next = c1;
    iotConfigHead->configEntry = configItemTest0;

    return iotConfigHead;
}