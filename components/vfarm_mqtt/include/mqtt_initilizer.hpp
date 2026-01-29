#ifndef MQTT_INITILIZER_HPP
#define MQTT_INITILIZER_HPP


#include "mqtt_client.h"
#include <stdatomic.h>
#include "freertos/queue.h"
#include <vector>
#include "mqtt_base.hpp"


namespace vfarm {



class MqttInitilizer: public MqttBase{
public:
    MqttInitilizer(const char* broker_url_i, const char* id);
    esp_mqtt_client_config_t mqtt_cfg;

    void init();
    void set_broker_uri(const char* broker_uri_i);


private:
    static void mqtt_event_handler(void *handler_args,
                                   esp_event_base_t base,
                                   int32_t event_id,
                                   void *event_data);

    // static esp_mqtt_client_handle_t mqtt_client;
};

}

#endif // MQTT_INITILIZER_HPP