#include "mqtt_client.h"
#include <stdatomic.h>
#include "freertos/queue.h"
#include <vector>
#include "mqtt_initilizer.hpp"
#include "esp_log.h"
#include "mqtt_base.hpp"

namespace vfarm {





MqttInitilizer::MqttInitilizer(const char* broker_url_i, const char* id)
{

    this->set_broker_uri(broker_url_i);
    mqtt_cfg.credentials.client_id = id;
    mqtt_cfg.session.disable_clean_session = true; // TO BE TESTED
    this->init();
}


void MqttInitilizer::init()
{
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

    // mqtt_rx_queue = xQueueCreate(10, sizeof(mqtt_message_t));
}



void MqttInitilizer::set_broker_uri(const char* broker_uri)
{
    this->broker_uri = broker_uri;
}



void vfarm::MqttInitilizer::mqtt_event_handler(void *handler_args,
                                 esp_event_base_t base,
                                 int32_t event_id,
                                 void *event_data)
{
    MqttInitilizer* self = static_cast<MqttInitilizer*>(handler_args);
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


            memcpy(msg.topic, event->topic, msg.topic_len);
            msg.topic[msg.topic_len] = '\0';
            memcpy(msg.payload, event->data, msg.payload_len);

            if (self->enable_debug_logs){
                ESP_LOGI(TAG, "Receiving data on topic: %.*s", msg.topic_len, msg.topic);
                ESP_LOGI(TAG, "Number of subscriber queues: %zu", self->subscriber_queues.size());
            }

            for (auto& sub : self->subscriber_queues) {
                if (self->enable_debug_logs){
                    ESP_LOGI(TAG, "Checking subscriber: %s, queue handle: %p", sub.topic, sub.queue);
                    ESP_LOGI(TAG, "Event topic: %s:", msg.topic);

                    ESP_LOGI(TAG, "Current queue length: %u / available slots: %u",
                            uxQueueMessagesWaiting(sub.queue),
                            uxQueueSpacesAvailable(sub.queue));
                }

                if (strcmp(sub.topic, msg.topic) == 0) {
                    if (xQueueSend(sub.queue, &msg, 0) == pdTRUE) {
                        if (self->enable_debug_logs) ESP_LOGI(TAG, "Message queued for topic: %s", sub.topic);
                    } else {
                        ESP_LOGW(TAG, "Queue full, message dropped for topic: %s", sub.topic);
                    }
                }
            }

            break;
        }

        default:
            break;




        }
    }
}