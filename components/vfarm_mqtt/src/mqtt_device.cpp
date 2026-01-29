#include "mqtt_device.hpp"
#include "esp_log.h"
#include <string.h>
#include "freertos/queue.h"
#include "mqtt_base.hpp"



namespace vfarm {





bool MqttDevice::is_connected()
{
    return mqtt_connected.load();
}

void MqttDevice::publish(const char* topic, void* data, size_t len, int qos, int retain)
// void MqttDevice::publish(const char* topic, const uint8_t* data, size_t len, int qos, int retain)
{

    if (!mqtt_connected || !mqtt_client) {
        ESP_LOGW(MqttDevice::TAG, "MQTT not connected, publish skipped");
        return;
    }

    esp_mqtt_client_publish(
        mqtt_client,
        topic,
        reinterpret_cast<const char*> (data),
        // (const char*)data,
        len,
        qos,
        retain
    );

}


// void AppMQTT::subscribe(const char* topic, int qos)
QueueHandle_t MqttDevice::subscribe(const char* topic, int qos, size_t queue_len)
{

    if (!mqtt_connected || !mqtt_client) {
        ESP_LOGW(MqttDevice::TAG, "MQTT not connected, subscribing skipped");
        return nullptr;
    }
    esp_mqtt_client_subscribe(mqtt_client, topic, qos);

    QueueHandle_t q = xQueueCreate(queue_len, sizeof(mqtt_message_t));
    subscriber_queue_t sub { q, topic };
    subscriber_queues.push_back(sub);

    device_subscriptions.push_back(sub);
    
    return q;
}

MqttBase::mqtt_message_t MqttDevice::get_msg(QueueHandle_t queue, TickType_t timeout) {
    mqtt_message_t msg = {};

    if (!queue) {
        ESP_LOGI(TAG, "Invalid que.");
        return msg;}

    if (xQueueReceive(queue, &msg, timeout) != pdTRUE) {
        // ESP_LOGI(TAG, "Timeout occurred. Not getting messages.");
    }

    return msg;

}

void MqttDevice::reconnect(TickType_t timeout ){
    while (!this->is_connected()) {
        ESP_LOGI(TAG, "Reconnecting to mqtt broker...");
        vTaskDelay(pdMS_TO_TICKS(500));
    }

}

}