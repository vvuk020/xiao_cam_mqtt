#ifndef CAM_C_HPP
#define CAM_C_HPP

#include <stdio.h>
#include "esp_camera.h"
#include "sensor.h"

#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    10
#define SIOD_GPIO_NUM    40
#define SIOC_GPIO_NUM    39

#define Y9_GPIO_NUM      48
#define Y8_GPIO_NUM      11
#define Y7_GPIO_NUM      12
#define Y6_GPIO_NUM      14
#define Y5_GPIO_NUM      16
#define Y4_GPIO_NUM      18
#define Y3_GPIO_NUM      17
#define Y2_GPIO_NUM      15
#define VSYNC_GPIO_NUM   38
#define HREF_GPIO_NUM    47
#define PCLK_GPIO_NUM    13



class CustCam {
public:
    const char *TAG = "WEB_CAM";
    camera_config_t camera_config;
    uint8_t *rgb888_image_data;


    CustCam();
    CustCam(camera_config_t cfg);

    esp_err_t init_cam();

    void convertRGB565toRGB888(const uint8_t *src, uint8_t *dst, int width, int height);


private:

};



#endif