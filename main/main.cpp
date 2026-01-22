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
#define SERVER_URL      "http://192.168.1.100:5000/upload" //"http://127.0.0.1:5000/upload"  // "http://192.168.1.177:5000/upload"
#define MQTT_BROKER_URI "mqtt://192.168.1.100:1883"        // <-- PC IP
#define MQTT_PIC        "random/number"
#define FRAME_QUEUE_SIZE 2  // number of frames in queue

extern "C"{


/* ================= TASKS ================= */
// Camera capture task
void camera_task(void* pvParameter) {
    const char* TAG = "CAM_TASK";
    while (1) {
        camera_fb_t* fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Failed to get frame");
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }
        else {
            ESP_LOGI(TAG, "Got picture! Size: %u bytes", fb->len);
        }


        // Put frame pointer into the queue (non-blocking)
        if (xQueueSend(frame_queue, &fb, 0) != pdTRUE) {
            ESP_LOGW(TAG, "Frame queue mqtt full, dropping frame");
            // esp_camera_fb_return(fb);
        }

        

        vTaskDelay(pdMS_TO_TICKS(500));
        // esp_camera_fb_return(fb);
    }
}


void mqtt_publisher(void* pvParameter) {
    AppMQTT* mqtt_t = (AppMQTT*) pvParameter;  // AppMQTT* mqtt_t = static_cast<AppMQTT*>(pvParameter);
    const char* TAG_PUB = "MQTT_PUBLISHER";
    camera_fb_t* fb;

    // AppMQTT mqtt(MQTT_BROKER_URI);
    // mqtt.init();

    // while (!mqtt_t->is_connected()) {
    //     ESP_LOGI(TAG_PUB, "Connecting...");
    //     vTaskDelay(pdMS_TO_TICKS(500));
    // }

    while (1) {

        if (!mqtt_t->is_connected()) {
            ESP_LOGI(TAG_PUB, "MQTT Not connected");
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        if (xQueueReceive(frame_queue, &fb, portMAX_DELAY) == pdTRUE) {
            mqtt_t->publish(
                "esp32/image",
                fb->buf,
                fb->len
            );

            esp_camera_fb_return(fb);

            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void mqtt_subscriber(void *pvParameter)
{
    const char* TAG_SUB = "MQTT_SUB_HEARTBEAT";
    const char* req_topic = "esp32/heartbeat/request";
    const char* resp_topic = "esp32/heartbeat/response";

    AppMQTT* mqtt_t = (AppMQTT*) pvParameter;  // AppMQTT* mqtt_t = static_cast<AppMQTT*>(pvParameter);

    while (!mqtt_t->is_connected()) {
        ESP_LOGI(TAG_SUB, "Connecting...");
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // mqtt_t->subscribe("esp32/heartbeat", 0);
    QueueHandle_t my_queue = mqtt_t->subscribe(req_topic, 0);  // queue length 5
    ESP_LOGI(TAG_SUB, "Queue created for topic %s: %p", req_topic, my_queue);

    AppMQTT::mqtt_message_t msg;
    while (true) {
        if (xQueueReceive(my_queue, &msg, portMAX_DELAY) == pdTRUE) {
            // ESP_LOGI(TAG_SUB, "Message received on topic: %s", msg.topic);

            mqtt_t->publish(resp_topic, (const uint8_t*)"ack", 3);
        }
    }


    // while (true) {
    //     if (xQueueReceive(mqtt_t->mqtt_rx_queue, &msg, portMAX_DELAY)) {
    //         ESP_LOGI(TAG_SUB, "Receiving messeges.");

    //         if (strcmp(msg.topic, "esp32/heartbeat") == 0) {
    //             ESP_LOGI(TAG_SUB, "Receiving on the topic: esp32/heartbeat");
    //             mqtt_t->publish("esp32/heartbeat/response", (const uint8_t*)"ack", 3);

    //         }
    //         else if (strcmp(msg.topic, "sub_topic2") == 0) {
    //             // handle_topic2(msg.payload, msg.payload_len);
    //         }
    //     }
    //     else { ESP_LOGI(TAG_SUB, "Not receiving messeges.");}
    // }
}


void mqtt_pic_request(void *pvParameter){

    const char* TAG_SUB = "MQTT_SUB_PICTURE";
    const char* req_topic = "esp32/picture/request";
    const char* resp_topic = "esp32/picture/response";
    
    AppMQTT* mqtt_t = (AppMQTT*) pvParameter;  // AppMQTT* mqtt_t = static_cast<AppMQTT*>(pvParameter);
    AppMQTT::mqtt_message_t msg;


    while (!mqtt_t->is_connected()) {
        ESP_LOGI(TAG_SUB, "Connecting...");
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // mqtt_t->subscribe(req_topic, 0);
    QueueHandle_t my_queue = mqtt_t->subscribe(req_topic, 0);  // queue length 5
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
        }
    }

}

/* ================= TASKS ================= */

void app_main(void)
{
    // Init camera
    WebCam web_cam(WIFI_SSID, WIFI_PASS, SERVER_URL);
    ESP_ERROR_CHECK(web_cam.init_cam());

    // Create queue
    frame_queue = xQueueCreate(FRAME_QUEUE_SIZE, sizeof(camera_fb_t*));
    frame_queue_http = xQueueCreate(FRAME_QUEUE_SIZE, sizeof(camera_fb_t*));

    // Create mqtt object
    static AppMQTT mqtt;  // Must be static!!!
    mqtt.set_broker_uri(MQTT_BROKER_URI);
    mqtt.init();   // <-- THIS CREATES mqtt_rx_queue

    // Start tasks
    // xTaskCreate(camera_task, "camera_task", 4096, NULL, 5, NULL);
    // xTaskCreate(mqtt_publisher, "mqtt_publisher", 4096, &mqtt, 5, NULL);
    xTaskCreate(mqtt_subscriber, "mqtt_subscriber", 8192, &mqtt, 5, NULL);
    xTaskCreate(mqtt_pic_request, "mqtt_pic_request", 8192, &mqtt, 5, NULL);

}
}