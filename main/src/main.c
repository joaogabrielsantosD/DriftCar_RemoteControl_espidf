/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"

#include "hardware_definitions.h"
#include "driver_definitions.h"

/*********************************************************************************/
//#define CHIP_INFO

static const char *TAG = "DriftCar_RemoteController";
static const char *VERSION = "0.0.0";
static uint8_t broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

void app_main(void);
/* Callbacks */
static void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status);
static void espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len);
void espnow_test(void *arg);
void potenciometer_test(void *arg);
void SOC_test(void *arg);

void app_main(void)
{
    ESP_LOGI(TAG, "Project Version: %s", VERSION);

    #ifdef CHIP_INFO
        /* Get Chip Information */
        esp_chip_info_t chip_info;
        uint32_t flash_size;
        esp_chip_info(&chip_info);

        printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
               CONFIG_IDF_TARGET,
               chip_info.cores,
               (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
               (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
               (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
               (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");
        unsigned major_rev = chip_info.revision / 100;
        unsigned minor_rev = chip_info.revision % 100;
        printf("silicon revision v%d.%d, ", major_rev, minor_rev);
        if (esp_flash_get_size(NULL, &flash_size) != ESP_OK)
            printf("Get flash size failed");
        printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
        (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
        printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());
        fflush(stdout);
    #endif

    gpio_config_t LED = {
        .pin_bit_mask = BIT(GPIO_NUM_2),
        .mode = GPIO_MODE_INPUT_OUTPUT
    };

    if (gpio_config(&LED) == ESP_OK) printf("DEU BOM\r\n");

    xTaskCreatePinnedToCore(&potenciometer_test, "pot", 4096, NULL, 5, NULL, 0);
    //xTaskCreatePinnedToCore(&SOC_test, "soc", 4096, NULL, 5, NULL, 0);
    //xTaskCreatePinnedToCore(&espnow_test, "wifi", 4096, NULL, 5, NULL, 1);

    while (1)
    {
        gpio_set_level(GPIO_NUM_2, gpio_get_level(GPIO_NUM_2) ^ 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* Core 0 */
void potenciometer_test(void *arg)
{
    adc1_config_width(SOC_WIDTH);
    adc1_config_channel_atten(MOTOR_DC_SPEED_ADC_CHANNEL, SOC_ATTENUATION);
    adc1_config_channel_atten(STEERING_WHELL_ADC_CHANNEL, SOC_ATTENUATION);
    //adc_set_data_inv(ADC_UNIT_1, true);

    while (true)
    {
        printf("Read Steering wheel potenciometer: %d\r\n", adc1_get_raw(STEERING_WHELL_ADC_CHANNEL));
        printf("Read DC Motor Speed potenciometer: %d\r\n", adc1_get_raw(MOTOR_DC_SPEED_ADC_CHANNEL));
        println();
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void SOC_test(void *arg)
{
    const int SAMPLES = 50;
    const float CALIBRATION_FACTOR = 1.068;

    adc1_config_width(SOC_WIDTH);
    adc1_config_channel_atten(SOC_ADC_CHANNEL, SOC_ATTENUATION);
    //adc_set_data_inv(ADC_UNIT_1, true);

    int val = 0;

    while (true)
    {
        for (int i = 0; i < SAMPLES; i++)
            val += adc1_get_raw(SOC_ADC_CHANNEL);
        val /= SAMPLES;
        
        uint16_t v = val;
        //uint16_t v = (uint16_t)~adc1_get_raw(SOC_ADC_CHANNEL) & 0b0000111111111111;
        //uint16_t v = (uint16_t)adc1_get_raw(SOC_ADC_CHANNEL);
        float vol = ((v * 9) / 4095.0) * CALIBRATION_FACTOR;
        uint8_t por = (int)((vol * 100) / 9);

        printf("Read: %d\r\n", v);
        printf("Volt: %.2f\r\n", vol);
        printf("Porc: %d\r\n", por);
        println();
        
        vTaskDelay(pdMS_TO_TICKS(750));
    }
}

/* Core 1 */
void espnow_test(void *arg)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(ESPNOW_WIFI_MODE));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_channel(CONFIG_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE));

    #if CONFIG_ESPNOW_ENABLE_LONG_RANGE
        ESP_ERROR_CHECK( esp_wifi_set_protocol(ESPNOW_WIFI_IF,                          \
            WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR));
    #endif

    uint8_t mac[6];
    esp_err_t mac_ret = esp_read_mac(mac, ESP_MAC_WIFI_STA);

    if (mac_ret == ESP_OK)
        printf("MAC: [0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    else
       printf("Failed to get MAC address, error: %d\n", ret); 
        
    /* Initialize ESPNOW and register sending and receiving callback function. */
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(espnow_send_cb));
    ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_recv_cb));

    #if CONFIG_ESPNOW_ENABLE_POWER_SAVE
        ESP_ERROR_CHECK(esp_now_set_wake_window(CONFIG_ESPNOW_WAKE_WINDOW));
        ESP_ERROR_CHECK(esp_wifi_connectionless_module_set_wake_interval(CONFIG_ESPNOW_WAKE_INTERVAL));
    #endif

    /* Add broadcast peer information to peer list. */
    esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL)
    {
        ESP_LOGE(TAG, "Malloc peer information fail");
        esp_now_deinit();
    }

    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = CONFIG_ESPNOW_CHANNEL;
    peer->ifidx   = ESPNOW_WIFI_IF;
    peer->encrypt = false;
    peer->priv    = NULL;
    memcpy(peer->peer_addr, broadcast_mac, ESP_NOW_ETH_ALEN);
    #if ESP_NOW_PMK
        /* Set primary master key. */
        peer->encrypt = true;
        uint8_t lmk[ESP_NOW_KEY_LEN] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22,
                                        0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00};
        ESP_ERROR_CHECK(esp_now_set_pmk((uint8_t *)lmk));
        memcpy(peer->lmk, lmk, ESP_NOW_KEY_LEN);
    #endif

    ESP_ERROR_CHECK(esp_now_add_peer(peer));
    free(peer);

    /* If MAC address does not exist in peer list, add it to peer list. */
    if (esp_now_is_peer_exist(broadcast_mac) == false) 
    {
        esp_now_peer_info_t *_peer = malloc(sizeof(esp_now_peer_info_t));
        if (_peer == NULL) 
        {
            ESP_LOGE(TAG, "Malloc peer information fail");
            vTaskDelete(NULL);
        }
        memset(_peer, 0, sizeof(esp_now_peer_info_t));
        _peer->channel = CONFIG_ESPNOW_CHANNEL;
        _peer->ifidx   = ESPNOW_WIFI_IF;
        _peer->encrypt = false;
        #if ESP_NOW_PMK
            /* Set primary master key. */
            _peer->encrypt = true;
            uint8_t _lmk[ESP_NOW_KEY_LEN] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22,
                                             0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00};
            ESP_ERROR_CHECK(esp_now_set_pmk((uint8_t *)_lmk));
            memcpy(_peer->lmk, _lmk, ESP_NOW_KEY_LEN);
        #endif
        memcpy(_peer->peer_addr, broadcast_mac, ESP_NOW_ETH_ALEN);
        ESP_ERROR_CHECK(esp_now_add_peer(_peer));
        free(_peer);
    }

    for (;;)
    {
        uint64_t time = esp_timer_get_time() / 1000;
        uint8_t len = sizeof(time);

        if (esp_now_send(broadcast_mac, (uint8_t *)&time, len) == ESP_OK)
            printf("Enviado mensagem [%lld] de tamanho [%d]\r\n", time, len);
        else
            ESP_LOGE(TAG, "ESP-NOW Send Error");


        delay(1000);
    }
}

/* Callbacks */
static void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    if (mac_addr == NULL) 
    {
        ESP_LOGE(TAG, "Send cb arg error");
        return;
    }

    ESP_LOGI(TAG, "Send ESP_NOW status: [%d]\r\n", status == ESP_NOW_SEND_SUCCESS ? 1 : 0);
}

static void espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    uint64_t rec_data = 0;
    uint8_t *mac_addr = recv_info->src_addr;
    uint8_t *des_addr = recv_info->des_addr;
    
    memcpy(&rec_data, (uint64_t*)data, sizeof(rec_data));

    if (mac_addr == NULL || data == NULL || len <= 0) 
    {
        ESP_LOGE(TAG, "Receive cb arg error");
        return;
    }

    printf("DATA: [");
    for (int i = 0; i < len; i++)
        printf("0x%x ", *(data + i));
    printf("]\r\n");

    printf("DES_ADDr: [0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]", des_addr[0], des_addr[1], des_addr[2], \
                                                        des_addr[3], des_addr[4], des_addr[5]);
    println();
    printf("SRC_ADDr: [0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]", mac_addr[0], mac_addr[1], mac_addr[2], \
                                                        mac_addr[3], mac_addr[4], mac_addr[5]);
    println();

    ESP_LOGI(TAG, "Receive sucess, mesage: [%lld] | len message: [%d]\r\n", rec_data, len);
}
