/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/i2c.h"

void app_main(void);

// EEPROM storage
#include "nvs_flash.h"
#include "nvs.h"
// ESP-NOW
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_now.h"

void app_main(void)
{
    printf("Hello world!\n");

    gpio_config_t LED = {
        .pin_bit_mask = BIT(GPIO_NUM_2),
        .mode = GPIO_MODE_INPUT_OUTPUT
    };

    if (gpio_config(&LED) == ESP_OK)
        printf("DEU BOM\r\n");

    

    while (1)
    {
        gpio_set_level(GPIO_NUM_2, gpio_get_level(GPIO_NUM_2) ^ 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}