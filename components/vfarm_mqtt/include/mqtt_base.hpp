#ifndef MQTT_BASE_HPP
#define MQTT_BASE_HPP


#include "mqtt_client.h"
#include <stdatomic.h>
#include "freertos/queue.h"
#include <vector>

#define MQTT_MAX_TOPIC_LEN 128
#define MQTT_MAX_PAYLOAD_LEN 2048   // adjust as needed


namespace vfarm {

class MqttBase{
public:
    static const char* TAG;
    const char* broker_uri;

    static atomic_bool mqtt_connected;
    static esp_mqtt_client_handle_t mqtt_client;

    static bool enable_debug_logs;
    // QueueHandle_t mqtt_rx_queue;
    struct subscriber_queue_t {
        QueueHandle_t queue;
        const char* topic;
    };
    
    struct mqtt_message_t {
        char topic[MQTT_MAX_TOPIC_LEN];
        size_t topic_len;
        uint8_t payload[MQTT_MAX_PAYLOAD_LEN];
        size_t payload_len;
    };





    static std::vector<subscriber_queue_t> subscriber_queues;


};

}
#endif // MQTT_BASE_HPP