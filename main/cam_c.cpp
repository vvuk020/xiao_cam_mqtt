#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"


#include "esp_camera.h"
#include "sensor.h"
#include "esp_log.h"

#include "cam_c.hpp"



CustCam::CustCam(){

    this->rgb888_image_data = NULL;
    ESP_ERROR_CHECK(this->init_cam());
}



CustCam::CustCam(camera_config_t cfg){
    
    this->rgb888_image_data = NULL;
    ESP_ERROR_CHECK(this->init_cam());
}

esp_err_t CustCam::init_cam(){
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
    camera_config.fb_count     = 1;
    camera_config.jpeg_quality = 12;
    camera_config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;



    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
    }
    return err;
}

void CustCam::convertRGB565toRGB888(const uint8_t *src, uint8_t *dst, int width, int height){
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

