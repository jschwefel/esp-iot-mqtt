#pragma once


typedef enum {
    GPIO_OUT_LEVEL_LOW =        0,
    GPIO_OUT_LEVEL_HIGH =       1,
} gpio_out_level_t;

typedef enum {
    IOT_GPIO_PULL_NEITHER =     0,
    IOT_GPIO_PULL_UP =          1,
    IOT_GPIO_PULL_DOWN =        2,
    IOT_GPIO_PULL_BOTH =        3,
} iot_gpio_pull_t;

// Need to refactor this as the pos/neg is alread
// determined by the interrupt type.
typedef enum {
    IOT_INTR_SWITCH_TOGGLE =         0,
    IOT_INTR_SWITCH_ONE_SHOT_POS =   1,
    IOT_INTR_SWITCH_ONE_SHOT_NEG =   2,
    IOT_INTR_SWITCH_TIMER_POS =      3,
    IOT_INTR_SWITCH_TIMER_NEG =      4,
} iot_simple_switch_type_t;

typedef enum {
    IOT_CONFIG_SIMPLE_SWITCH =  0,


    IOT_CONFIG_DUMMY_TEST =     255,
} iot_config_item_type_t;


typedef enum {
    IOT_MQTT_QOS_NO_TOPIC =         0,
    IOT_MQTT_QOS_MOST_ONCE =        1,
    IOT_MQTT_QOS_LEAST_ONCE =       2,
    IOT_MQTT_QOS_ONLY_ONCE =        3,
    
} iot_mqtt_qos_t;