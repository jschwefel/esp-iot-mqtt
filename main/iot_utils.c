#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "iot_globals.h"
#include "esp_mac.h"
#include "driver/gpio.h"

char* get_mac_address_half_low() {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_BASE);
    char *macLastSix = malloc(7);
    sprintf(macLastSix, "%02x%02x%02x", mac[3], mac[4], mac[5]);
    return macLastSix;
}

bool gpio_normailized_state(bool inverted, uint32_t gpioPin) {
    bool pinBool = gpio_get_level(gpioPin) == 0 ? false : true;
    if (inverted) {
        return !pinBool;
    } else {
        return pinBool;
    }
}

char* concat(const char *s1, const char *s2) {
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

void freeConfigList(iot_config_linked_list_t* head) {
   iot_config_linked_list_t* tmp;
   while (head != NULL)
    {
       tmp = head;
       head = head->next;
       free(tmp);
    }
}

iot_config_linked_list_t* getLastEntry(iot_config_linked_list_t* linkedList) {
    iot_config_linked_list_t* tmp = calloc(1, sizeof(iot_config_linked_list_t)); 
    tmp = linkedList;
    while(true) {
        if(tmp->next == NULL) {
            break;
        }
        tmp = tmp->next;
    }
    return tmp;
}