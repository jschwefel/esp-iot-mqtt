#pragma once


typedef enum 
{
    GPIO_OUT_LEVEL_LOW =        0,
    GPIO_OUT_LEVEL_HIGH =       1,
} gpio_out_level_t;

typedef enum
{
    IOT_GPIO_PULL_UP =          0,
    IOT_GPIO_PULL_DOWN =        1,
    IOT_GPIO_PULL_BOTH =        2,
} iot_gpio_pull_t;

typedef enum
{
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
