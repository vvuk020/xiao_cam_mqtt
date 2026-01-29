# ESP32 XIAO Camera MQTT Client

This project implements an **ESP32 (Seeed Studio XIAO ESP32-S3 + camera)** application that captures images from the onboard camera and publishes them over **MQTT** on request. It also provides a simple **heartbeat mechanism** to monitor device availability.

The design is inspired by projects like [`xiao_cam_mqtt`](https://github.com/vvuk020/xiao_cam_mqtt), but is structured as a reusable ESP-IDF component-based application with custom Wi‑Fi and MQTT abstractions.

---

## Features

* Camera capture using `esp32-camera`
* MQTT-based request/response image transfer
* Heartbeat request/ack mechanism via MQTT
* Wi‑Fi STA initialization and connection handling
* Modular design using ESP-IDF components
* FreeRTOS-based multitasking

---

## Project Structure

```
.
├── main/
│   ├── main.cpp          # app_main, FreeRTOS tasks
│   ├── cam_c.cpp         # Camera implementation
│   └── cam_c.hpp         # Camera class definition & pinout
│
├── components/
│   └── vfarm_mqtt/
│       ├── include/
│       │   ├── mqtt_base.hpp
│       │   ├── mqtt_device.hpp
│       │   ├── mqtt_initilizer.hpp
│       │   └── wifi_c.hpp
│       └── src/
│           ├── mqtt_base.cpp
│           ├── mqtt_device.cpp
│           ├── mqtt_initilizer.cpp
│           └── wifi_c.cpp
│
├── CMakeLists.txt
└── idf_component.yml
```

---

## Requirements

* ESP-IDF **v4.1 or newer**
* ESP32 with camera support (tested with **XIAO ESP32-S3**)
* MQTT broker (e.g. Mosquitto)
* Wi‑Fi network

ESP-IDF dependency:

* `espressif/esp32-camera`

---

## Camera Configuration

The camera is wrapped in the `CustCam` class. Pin definitions are located in `cam_c.hpp` and configured for the XIAO ESP32-S3 camera module.

Default configuration:

* Pixel format: `JPEG`
* Frame size: `FRAMESIZE_HD`
* JPEG quality: `12`
* Frame buffer count: `1`

Camera initialization happens automatically when the `CustCam` object is constructed.

---

## 📡 MQTT Topics

| Purpose            | Topic                      | Direction |
| ------------------ | -------------------------- | --------- |
| Picture request    | `esp32/picture/request`    | RX        |
| Picture response   | `esp32/picture/response`   | TX        |
| Heartbeat request  | `esp32/heartbeat/request`  | RX        |
| Heartbeat response | `esp32/heartbeat/response` | TX        |

---

## Runtime Behavior

### Heartbeat Task (`hbeat_task`)

* Subscribes to `esp32/heartbeat/request`
* On any incoming message, replies with `"ack"`
* Automatically reconnects if MQTT connection drops

### Camera Task (`cam_task`)

* Subscribes to `esp32/picture/request`
* On request:

  1. Captures a camera frame
  2. Publishes raw JPEG buffer to `esp32/picture/response`
  3. Returns frame buffer to the driver

---

## Wi‑Fi Setup

Wi‑Fi credentials are defined in `main.cpp`:

```c
#define WIFI_SSID "your_ssid"
#define WIFI_PASS "your_password"
```

Wi‑Fi is initialized via the `WifiCust` class and blocks until a connection is established.

---

## Build & Flash

```bash
idf.py set-target esp32s3
idf.py build
idf.py flash monitor
```

---

## Design Notes

* MQTT is implemented as a **shared static client** with per-device subscription queues
* Each task owns its own MQTT queue for clean separation
* Messages are routed in the MQTT event handler and dispatched to matching subscriber queues
* JPEG images are sent as **raw binary payloads**

---

## Limitations

* No image chunking (large frames may exceed broker limits)
* Single frame buffer
* No TLS / authentication on MQTT

---

## License

MIT License (or specify your preferred license)