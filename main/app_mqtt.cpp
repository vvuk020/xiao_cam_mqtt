#include "app_mqtt.hpp"
#include "esp_log.h"
#include <string.h>
#include "freertos/queue.h"
#include <atomic>

/* static members */
atomic_bool AppMQTT::mqtt_connected = false;
esp_mqtt_client_handle_t AppMQTT::mqtt_client = NULL;
const char* AppMQTT::TAG = "APP_MQTT";
std::vector<AppMQTT::subscriber_queue_t> AppMQTT::subscriber_queues;


AppMQTT::AppMQTT()
    : broker_uri(nullptr)
{
}

AppMQTT::AppMQTT(const char* broker_uri)
    : broker_uri(broker_uri)
{
}

AppMQTT::~AppMQTT()
{
}



void AppMQTT::set_broker_uri(const char* broker_uri)
{
    this->broker_uri = broker_uri;
}

void AppMQTT::init()
{
    esp_mqtt_client_config_t mqtt_cfg;
    memset(&mqtt_cfg, 0, sizeof(mqtt_cfg));

    mqtt_cfg.broker.address.uri = broker_uri;

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(
        mqtt_client,
        (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID,
        mqtt_event_handler,
        this
    );

    esp_mqtt_client_start(mqtt_client);

    mqtt_rx_queue = xQueueCreate(10, sizeof(mqtt_message_t));

}

bool AppMQTT::is_connected()
{
    return mqtt_connected.load();
}

void AppMQTT::publish(const char* topic, const uint8_t* data, size_t len, int qos, int retain)
{
    if (!mqtt_connected || !mqtt_client) {
        ESP_LOGW(AppMQTT::TAG, "MQTT not connected, publish skipped");
        return;
    }

    esp_mqtt_client_publish(
        mqtt_client,
        topic,
        (const char*)data,
        len,
        qos,
        retain
    );
}

// void AppMQTT::subscribe(const char* topic, int qos)
QueueHandle_t AppMQTT::subscribe(const char* topic, int qos, size_t queue_len)
{
    if (!mqtt_connected || !mqtt_client) {
        ESP_LOGW(AppMQTT::TAG, "MQTT not connected, subscribing skipped");
        return nullptr;
    }
    esp_mqtt_client_subscribe(mqtt_client, topic, qos);

    QueueHandle_t q = xQueueCreate(queue_len, sizeof(mqtt_message_t));
    subscriber_queue_t sub { q, topic };
    subscriber_queues.push_back(sub);
    
    return q;
}

void AppMQTT::mqtt_event_handler(void *handler_args,
                                 esp_event_base_t base,
                                 int32_t event_id,
                                 void *event_data)
{
    AppMQTT* self = static_cast<AppMQTT*>(handler_args);
    auto event = static_cast<esp_mqtt_event_handle_t>(event_data);

    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            mqtt_connected = true;
            ESP_LOGI(TAG, "MQTT connected");
            break;

        case MQTT_EVENT_DISCONNECTED:
            mqtt_connected = false;
            ESP_LOGI(TAG, "MQTT disconnected");
            break;
        
        case MQTT_EVENT_DATA: {
            mqtt_message_t msg = {};

            msg.topic_len = event->topic_len;
            msg.payload_len = event->data_len;

            // if (msg.topic_len >= MQTT_MAX_TOPIC_LEN ||
            //     msg.payload_len >= MQTT_MAX_PAYLOAD_LEN) {
            //     ESP_LOGW(TAG, "MQTT message too large, dropped");
            //     break;
            // }

            memcpy(msg.topic, event->topic, msg.topic_len);
            msg.topic[msg.topic_len] = '\0';

            memcpy(msg.payload, event->data, msg.payload_len);

            // if (xQueueSend(self->mqtt_rx_queue, &msg, 0) != pdTRUE) {
            //     ESP_LOGW(TAG, "MQTT RX queue full, dropped message");
            // }



            // ESP_LOGI(TAG, "Receving data");
            // for (auto& sub : self->subscriber_queues) {
            //     if (strcmp(sub.topic, event->topic) == 0) {
            //         xQueueSend(sub.queue, &msg, 0);  // non-blocking
            //     }
            // }


            ESP_LOGI(TAG, "Receiving data on topic: %.*s", msg.topic_len, msg.topic);
            ESP_LOGI(TAG, "Number of subscriber queues: %zu", self->subscriber_queues.size());

            bool matched = false;
            for (auto& sub : self->subscriber_queues) {
                ESP_LOGI(TAG, "Checking subscriber: %s, queue handle: %p", sub.topic, sub.queue);
                ESP_LOGI(TAG, "Event topic: %s:", msg.topic);

                ESP_LOGI(TAG, "Current queue length: %u / available slots: %u",
                        uxQueueMessagesWaiting(sub.queue),
                        uxQueueSpacesAvailable(sub.queue));

                if (strcmp(sub.topic, msg.topic) == 0) {
                    if (xQueueSend(sub.queue, &msg, 0) == pdTRUE) {
                        ESP_LOGI(TAG, "Message queued for topic: %s", sub.topic);
                    } else {
                        ESP_LOGW(TAG, "Queue full, message dropped for topic: %s", sub.topic);
                    }
                    matched = true;
                }
            }

            break;
        }

        default:
            break;
    }
}
