idf_component_register(SRCS "main.cpp" "app_webcam.cpp" "app_mqtt.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi esp_netif nvs_flash esp_event esp_http_client mqtt
)
