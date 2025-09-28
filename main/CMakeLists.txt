idf_component_register(SRCS "Smart_Embed.c"
                    INCLUDE_DIRS "."
                    REQUIRES oled_driver ultrasonic_sensor http_server_app esp32c3_wifi sd_card_spi)
