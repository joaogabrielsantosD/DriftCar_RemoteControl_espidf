idf_component_register(SRCS "src/main.c" "lib/lcd/src/lcd.c"
                    INCLUDE_DIRS "." "include" "lib/lcd"
                    REQUIRES driver spi_flash nvs_flash esp_event esp_netif esp_wifi ssd1306 esp_timer)
