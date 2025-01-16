idf_component_register(SRCS "src/main.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver spi_flash nvs_flash esp_event esp_netif esp_wifi ssd1306)
