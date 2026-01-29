#ifndef MQTT_DEVICE_HPP
#define MQTT_DEVICE_HPP


#include "mqtt_client.h"
#include "freertos/queue.h"
#include <vector>
#include "mqtt_base.hpp"

namespace vfarm {



class MqttDevice: public MqttBase{
public:
    QueueHandle_t queue;
    std::vector<subscriber_queue_t> device_subscriptions;
    
    // MqttDevice();


    void publish(const char* topic, void* data, size_t len, int qos = 0, int retain = 0);
    // void publish(const char* topic, const uint8_t* data, size_t len, int qos = 0, int retain = 0);
    // void subscribe(const char* topic, int qos);
    QueueHandle_t subscribe(const char* topic, int qos, size_t queue_len = 2);
    mqtt_message_t get_msg(QueueHandle_t queue, TickType_t timeout = pdTRUE);
    bool is_connected();
    void reconnect(TickType_t timeout = pdTRUE);
};

}

#endif // MQTT_DEVICE_HPP