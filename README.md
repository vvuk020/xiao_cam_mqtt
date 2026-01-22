# XIAO CAMERA MQTT System

A FreeRTOS-based XIAO CAM (ESP32-S3) camera system that captures images and publishes them over MQTT, with support for request/response messaging and optional HTTP upload. Designed for remote monitoring, IoT projects, and lightweight surveillance.

## Features
* Camera integration with `esp_camera`
* Captures frames and queues them for processing
* Publishes images via MQTT to a broker
* Subscribes to request topics for heartbeat and picture retrieval
* Optional HTTP upload support
* FreeRTOS task-based architecture for concurrent operations
* Logging with `ESP_LOG` for debugging

## Installation

1. Clone this repository:

```bash
git clone https://github.com/vvuk020/xiao_cam_mqtt.git
cd xiao_cam_mqtt
```

2. Open the project in **ESP-IDF** (v5.x recommended)

3. Configure your Wi-Fi and MQTT settings in `main.cpp`:

```cpp
#define WIFI_SSID       "your_ssid"
#define WIFI_PASS       "your_password"
#define MQTT_BROKER_URI "mqtt://broker_ip:1883"
#define SERVER_URL      "http://server_ip:5000/upload"
```

4. Build and flash:

```bash
idf.py build
idf.py flash monitor
```

## Usage

Once flashed, the ESP32 will:

* Connect to Wi-Fi and MQTT broker
* Start camera and create FreeRTOS queues
* Start MQTT tasks:

  * **Subscriber**: handles heartbeat requests (`esp32/heartbeat/request`)
  * **Picture request**: handles picture requests (`esp32/picture/request`)
  * **Publisher**: publishes frames to `esp32/image`
* Optionally upload images to a server via HTTP POST

MQTT messages are JSON-compatible, and images are sent as raw byte payloads.

### Example MQTT Topics

| Topic                      | Direction | Description                      |
| -------------------------- | --------- | -------------------------------- |
| `esp32/image`              | Publish   | Camera frames                    |
| `esp32/picture/request`    | Subscribe | Request single picture           |
| `esp32/picture/response`   | Publish   | Response with requested picture  |
| `esp32/heartbeat/request`  | Subscribe | Heartbeat ping                   |
| `esp32/heartbeat/response` | Publish   | Heartbeat acknowledgment (`ack`) |

## Folder Structure

```
main.cpp                   # Main FreeRTOS application
app_webcam.hpp/cpp          # Camera initialization and capture logic
app_mqtt.hpp/cpp            # MQTT client wrapper for publishing/subscribing
components/                 # Additional ESP-IDF components
```

## Customization

* **Wi-Fi & MQTT Settings:** Adjust `WIFI_SSID`, `WIFI_PASS`, `MQTT_BROKER_URI` in `main.cpp`.
* **Queue Size:** Modify `FRAME_QUEUE_SIZE` to control how many frames are buffered.
* **HTTP Upload:** Set `SERVER_URL` to enable POST requests of captured frames.
* **MQTT Topics:** You can add or modify topics in `mqtt_subscriber` and `mqtt_pic_request` tasks.

## License

MIT License — free to use and modify.