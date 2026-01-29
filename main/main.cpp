#include "esp_log.h"
#include "cam_c.hpp"

#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "wifi_c.hpp"
#include "mqtt_base.hpp"
#include "mqtt_initilizer.hpp"
#include "mqtt_device.hpp"




#define WIFI_SSID           "das_labor"
#define WIFI_PASS           "rosrosros"
#define SERVER_URL          "http://192.168.1.100:5000/upload" //"http://127.0.0.1:5000/upload"  // "http://192.168.1.177:5000/upload"
#define MQTT_BROKER_URI     "mqtt://192.168.1.100:1883"        // <-- PC IP
#define CLIENT_ID           "mqtt://192.168.1.100:1883"        // <-- PC IP
#define pic_req_topic       "esp32/picture/request"
#define pic_resp_topic      "esp32/picture/response"
#define hbeat_req_topic     "esp32/heartbeat/request"
#define hbeat_resp_topic    "esp32/heartbeat/response"
  

extern "C"{
/* ================= TASKS ================= */
// Heatbeat task
void hbeat_task(void* pvParameter) {
    const char* TAG_SUB = "hbeat_task";
    vfarm::MqttDevice mqtt_device;
    
    auto hbeat_q = mqtt_device.subscribe(hbeat_req_topic, 1);
    ESP_LOGI(TAG_SUB, "Queue created for topic %s: %p", hbeat_req_topic, hbeat_q);

     while (1) {
        auto msg = mqtt_device.get_msg(hbeat_q);

        if (msg.payload_len > 0){
            ESP_LOGI(TAG_SUB, "Message received on topic: %s", msg.topic);
            mqtt_device.publish(hbeat_resp_topic, (void*)"ack", sizeof("ack") - 1);

        }

        if (!mqtt_device.is_connected()) {
            ESP_LOGI(TAG_SUB, "Connecting to mqtt broker...");
            mqtt_device.reconnect();
            hbeat_q = mqtt_device.subscribe(hbeat_req_topic, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Camera task
void cam_task(void* pvParameter){
    const char* TAG_SUB = "cam_task";
    vfarm::MqttDevice mqtt_device;

    auto cam_q = mqtt_device.subscribe(pic_req_topic, 0);
    
    ESP_LOGI(TAG_SUB, "Queue created for topic %s: %p", pic_req_topic, cam_q);

    while (1) {
        auto msg = mqtt_device.get_msg(cam_q);
        if (msg.payload_len > 0){
            ESP_LOGI(TAG_SUB, "Message received on topic: %s", msg.topic);
            
            camera_fb_t* fb = esp_camera_fb_get();
            ESP_LOGI(TAG_SUB, "Got picture! Size: %u bytes", fb->len);

            if (!fb) {
                ESP_LOGE(TAG_SUB, "Failed to get frame");
                vTaskDelay(pdMS_TO_TICKS(50));
                continue;
            }
            mqtt_device.publish(pic_resp_topic, fb->buf, fb->len);
            esp_camera_fb_return(fb);

        }
       
        if (!mqtt_device.is_connected()){
            ESP_LOGI(TAG_SUB, "Connecting to mqtt broker...");
            mqtt_device.reconnect();
            cam_q = mqtt_device.subscribe(pic_req_topic, 1);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
/* ================= TASKS ================= */



void app_main(void)
{
    const char* TAG = "ESP";
    
    vfarm::WifiCust wifi_init(WIFI_SSID, WIFI_PASS);
    vTaskDelay(pdMS_TO_TICKS((500)));
    if (wifi_init.check_status()) ESP_LOGI(TAG, "Connected to wifi.");

    CustCam cam;
    vfarm::MqttInitilizer mqtt_init(MQTT_BROKER_URI, CLIENT_ID);
    vfarm::MqttBase::enable_debug_logs = true;

    while (!vfarm::MqttBase::mqtt_connected.load()) {
        ESP_LOGI(TAG, "Connecting to mqtt broker...");
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    xTaskCreate(hbeat_task, "hbeat_task", 8192, NULL, 24, NULL);
    xTaskCreate(cam_task, "cam_task", 8192, NULL, 24, NULL);


}
}