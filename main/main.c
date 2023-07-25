/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "iot_globals.h"
#include "iot_wifi.h"





void app_main(void)
{
    globals_init();
    init_wifi();
    
    return;
}
