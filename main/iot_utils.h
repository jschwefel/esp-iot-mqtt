/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#pragma once

#include <stdio.h>
#include <stdbool.h>
#include "iot_structs.h"



char* get_mac_address_half_low();
uint32_t gpio_normailized_state(bool inverted, uint32_t gpioPin);
char* concat(const char *s1, const char *s2);
iot_config_linked_list_t* getLastEntry(iot_config_linked_list_t* linkedList);
int tokenCount(char *string, char *delimeter);
void stringSplitter(char *string, char *delimeter, char *tokenArray[]);
void str_replace(char *target, const char *needle, const char *replacement);
void spiffs_dir(char* directory);
const char* get_filename_ext(const char *filename);