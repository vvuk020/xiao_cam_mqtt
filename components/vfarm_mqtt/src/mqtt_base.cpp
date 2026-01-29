


#include "mqtt_client.h"
#include <stdatomic.h>
#include "freertos/queue.h"
#include <vector>
#include "mqtt_base.hpp"

namespace vfarm {



/* static members */
atomic_bool MqttBase::mqtt_connected = false;
esp_mqtt_client_handle_t MqttBase::mqtt_client = NULL;
const char* MqttBase::TAG = "APP_MQTT";
std::vector<MqttBase::subscriber_queue_t> MqttBase::subscriber_queues;
bool MqttBase::enable_debug_logs = false;

}