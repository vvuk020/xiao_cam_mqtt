#ifndef APP_MQTT_HPP
#define APP_MQTT_HPP


#include "mqtt_client.h"
#include <stdatomic.h>
#include "freertos/queue.h"
#include <vector>

#define MQTT_MAX_TOPIC_LEN 128
#define MQTT_MAX_PAYLOAD_LEN 2048   // adjust as needed




class AppMQTT {
public:
    // const char *TAG = "APP_MQTT";
    static const char* TAG;
    QueueHandle_t mqtt_rx_queue;

    struct subscriber_queue_t {
        QueueHandle_t queue;
        const char* topic;
    };
    static std::vector<subscriber_queue_t> subscriber_queues;
    
    struct mqtt_message_t {
    char topic[MQTT_MAX_TOPIC_LEN];
    size_t topic_len;

    uint8_t payload[MQTT_MAX_PAYLOAD_LEN];
    size_t payload_len;
    };


    AppMQTT();
    AppMQTT(const char* broker_uri);
    ~AppMQTT();

    void set_broker_uri(const char* broker_uri);
    void init();
    bool is_connected();
    void publish(const char* topic, const uint8_t* data, size_t len, int qos = 0, int retain = 0);
    // void subscribe(const char* topic, int qos);
    QueueHandle_t subscribe(const char* topic, int qos, size_t queue_len = 10);
private:
    static void mqtt_event_handler(void *handler_args,
                                   esp_event_base_t base,
                                   int32_t event_id,
                                   void *event_data);

    static atomic_bool mqtt_connected;
    static esp_mqtt_client_handle_t mqtt_client;

    const char* broker_uri;
};

#endif
