#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_camera.h"
#include "esp_log.h"
#include "app_webcam.hpp"
#include "app_mqtt.hpp"


QueueHandle_t frame_queue;
QueueHandle_t frame_queue_http;

#define WIFI_SSID       "das_labor"
#define WIFI_PASS       "rosrosros"
#define MQTT_BROKER_URI "mqtt://192.168.1.100:1883"        // <-- PC IP
#define MQTT_PIC        "random/number"
#define FRAME_QUEUE_SIZE 2  // number of frames in queue

extern "C"{


/* ================= TASKS ================= */

void mqtt_heartbeat(void *pvParameter)
{
    const char* TAG_SUB = "MQTT_SUB_HEARTBEAT";
    const char* req_topic = "esp32/heartbeat/request";
    const char* resp_topic = "esp32/heartbeat/response";

    AppMQTT* mqtt_t = (AppMQTT*) pvParameter;

    while (!mqtt_t->is_connected()) {
        ESP_LOGI(TAG_SUB, "Connecting...");
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    QueueHandle_t my_queue = mqtt_t->subscribe(req_topic, 0);  // queue length 5
    ESP_LOGI(TAG_SUB, "Queue created for topic %s: %p", req_topic, my_queue);

    AppMQTT::mqtt_message_t msg;
    while (true) {
        if (xQueueReceive(my_queue, &msg, portMAX_DELAY) == pdTRUE) {
            mqtt_t->publish(resp_topic, (const uint8_t*)"ack", 3);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void mqtt_pic_request(void *pvParameter){

    const char* TAG_SUB = "MQTT_SUB_PICTURE";
    const char* req_topic = "esp32/picture/request";
    const char* resp_topic = "esp32/picture/response";
    
    AppMQTT* mqtt_t = (AppMQTT*) pvParameter;
    AppMQTT::mqtt_message_t msg;
    while (!mqtt_t->is_connected()) {
        ESP_LOGI(TAG_SUB, "Connecting...");
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    QueueHandle_t my_queue = mqtt_t->subscribe(req_topic, 0);
    ESP_LOGI(TAG_SUB, "Queue created for topic %s: %p", req_topic, my_queue);

    while (true) {
        if (xQueueReceive(my_queue, &msg, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG_SUB, "Message received on topic: %s", msg.topic);

            camera_fb_t* fb = esp_camera_fb_get();
            if (!fb) {
                ESP_LOGE(TAG_SUB, "Failed to get frame");
                vTaskDelay(pdMS_TO_TICKS(50));
                continue;
            }
            ESP_LOGI(TAG_SUB, "Got picture! Size: %u bytes", fb->len);

            mqtt_t->publish(resp_topic, fb->buf, fb->len);

            esp_camera_fb_return(fb);
            vTaskDelay(pdMS_TO_TICKS(100));
        
        }
    }
}

/* ================= TASKS ================= */

void app_main_simple_sub(void)
{
    // Init camera
    WebCam web_cam(WIFI_SSID, WIFI_PASS);
    ESP_ERROR_CHECK(web_cam.init_cam());

    // Create queue
    frame_queue = xQueueCreate(FRAME_QUEUE_SIZE, sizeof(camera_fb_t*));
    frame_queue_http = xQueueCreate(FRAME_QUEUE_SIZE, sizeof(camera_fb_t*));

    // Create mqtt object
    AppMQTT& mqtt = AppMQTT::instance();
    mqtt.set_broker_uri(MQTT_BROKER_URI);
    mqtt.init();

    // Start tasks
    xTaskCreate(mqtt_heartbeat, "mqtt_heartbeat", 8192, &mqtt, 5, NULL);
    xTaskCreate(mqtt_pic_request, "mqtt_pic_request", 8192, &mqtt, 5, NULL);
}
}