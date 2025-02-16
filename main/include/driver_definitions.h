#ifndef DRIVER_DEFS_H
#define DRIVER_DEFS_H

#include "sdkconfig.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "math.h"
// EEPROM storage
#include "nvs_flash.h"
#include "nvs.h"
// ESP-NOW
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_now.h"
// Oled 128x64
#include "ssd1306.h"
#include "font8x8_basic.h" 

/* ADC channel */
#define MOTOR_DC_SPEED_ADC_CHANNEL ADC1_CHANNEL_4
#define STEERING_WHELL_ADC_CHANNEL ADC1_CHANNEL_5
#define SOC_ADC_CHANNEL            ADC1_CHANNEL_6
#define SOC_WIDTH                  ADC_WIDTH_BIT_12
#define SOC_ATTENUATION            ADC_ATTEN_DB_12

/* ESP-NOW Definitions */
#if CONFIG_ESPNOW_WIFI_MODE_STATION
    #define ESPNOW_WIFI_MODE WIFI_MODE_STA
    #define ESPNOW_WIFI_IF   ESP_IF_WIFI_STA
#else
    #define ESPNOW_WIFI_MODE WIFI_MODE_AP
    #define ESPNOW_WIFI_IF   ESP_IF_WIFI_AP
#endif

#if CONFIG_ESPNOW_ENABLE_POWER_SAVE
    #define CONFIG_ESPNOW_WAKE_WINDOW   1000
    #define CONFIG_ESPNOW_WAKE_INTERVAL 1000
#endif  

#define CONFIG_ESPNOW_CHANNEL 1

#endif