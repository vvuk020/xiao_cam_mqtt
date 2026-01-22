#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_err.h"


#include "esp_camera.h"
#include "sensor.h"
#include "esp_log.h"

#include "app_webcam.hpp"


WebCam::WebCam(const char *wifi_ssid, const char *wifi_pass, const char *server_url)
    : wifi_ssid(wifi_ssid), wifi_pass(wifi_pass), server_url(server_url) {
    
    this->init_wifi();
    this->rgb888_image_data = NULL;
}

WebCam::WebCam(){
    
    this->rgb888_image_data = NULL;
}


WebCam::~WebCam() {
    
}

void WebCam::init_wifi(){
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();
}


esp_err_t WebCam::init_cam(){
    camera_config.ledc_channel = LEDC_CHANNEL_0,
    camera_config.ledc_timer   = LEDC_TIMER_0,


    camera_config.pin_pwdn     = PWDN_GPIO_NUM;
    camera_config.pin_reset    = RESET_GPIO_NUM;
    camera_config.pin_xclk     = XCLK_GPIO_NUM;
    camera_config.pin_sccb_sda = SIOD_GPIO_NUM;
    camera_config.pin_sccb_scl = SIOC_GPIO_NUM;

    camera_config.pin_d7       = Y9_GPIO_NUM;
    camera_config.pin_d6       = Y8_GPIO_NUM;
    camera_config.pin_d5       = Y7_GPIO_NUM;
    camera_config.pin_d4       = Y6_GPIO_NUM;
    camera_config.pin_d3       = Y5_GPIO_NUM;
    camera_config.pin_d2       = Y4_GPIO_NUM;
    camera_config.pin_d1       = Y3_GPIO_NUM;
    camera_config.pin_d0       = Y2_GPIO_NUM;

    camera_config.pin_vsync    = VSYNC_GPIO_NUM;
    camera_config.pin_href     = HREF_GPIO_NUM;
    camera_config.pin_pclk     = PCLK_GPIO_NUM;

    camera_config.xclk_freq_hz = 20000000;
    camera_config.pixel_format = PIXFORMAT_JPEG;  // PIXFORMAT_RGB565
    camera_config.frame_size   = FRAMESIZE_HD;    // FRAMESIZE_QQVGA;
    camera_config.fb_count     = 2;
    camera_config.jpeg_quality = 12;
    camera_config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;



    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
    }
    return err;
}

void WebCam::set_wifi_creds(const char* wifi_ssid, const char *wifi_pass){
    
    this->wifi_ssid = wifi_ssid;
    this->wifi_pass = wifi_pass;
}

void WebCam::set_server_url(const char *server_url){
    this->server_url = server_url;
}

void WebCam::wifi_init_sta(){
    {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {};
    strncpy((char *)wifi_config.sta.ssid, wifi_ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, wifi_pass, sizeof(wifi_config.sta.password));

    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to Wi-Fi...");
    ESP_ERROR_CHECK(esp_wifi_connect());
    }
}

void WebCam::convertRGB565toRGB888(const uint8_t *src, uint8_t *dst, int width, int height){
    const uint16_t *src16 = (const uint16_t *)src;
    int pixels = width * height;
    for (int i = 0; i < pixels; i++) {
        
        uint16_t p = src16[i];
        uint8_t r = (p >> 11) & 0x1F;
        uint8_t g = (p >> 5)  & 0x3F;
        uint8_t b =  p        & 0x1F;

        dst[i * 3 + 0] = (r << 3) | (r >> 2);
        dst[i * 3 + 1] = (g << 2) | (g >> 4);
        dst[i * 3 + 2] = (b << 3) | (b >> 2);
    }
}

esp_err_t WebCam::send_frame(uint8_t *buf, size_t len)
{
    esp_http_client_config_t config = {0};

    config.url = server_url;
    config.method = HTTP_METHOD_POST;

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_header(client, "Content-Type", "application/octet-stream");
    esp_http_client_set_post_field(client, (const char *)buf, len);

    esp_err_t err = esp_http_client_perform(client);


    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Frame sent: HTTP %d",
                 esp_http_client_get_status_code(client));
    } else {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
    }

    ESP_LOGI(TAG, "Sending frame of size: %zu from address: %p", len, buf);
    esp_http_client_cleanup(client);

    return err;


}

bool WebCam::server_is_up() {
    if (!server_url) return false;

    esp_http_client_config_t config = {};
    config.url = server_url;
    config.method = HTTP_METHOD_GET;
    config.timeout_ms = 1000;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) return false;

    esp_err_t err = esp_http_client_perform(client);
    int status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);

    return (err == ESP_OK && status == 200);
}

