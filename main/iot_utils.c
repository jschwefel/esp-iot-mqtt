#include <stdio.h>
#include <string.h>
#include "esp_mac.h"

char* get_mac_address_half_low()
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_BASE);
    char *macLastSix = malloc(7);
    sprintf(macLastSix, "%02x%02x%02x", mac[3], mac[4], mac[5]);
    return macLastSix;
}