import base64
import io
import time
import paho.mqtt.client as mqtt
from PIL import Image

BROKER = "192.168.1.163"
PORT = 1883
REQ_TOPIC = "ESP32_CAM_2/picture/request"
RESP_TOPIC = "ESP32_CAM_2/picture/response"

image_data = None

def on_message(client, userdata, msg):
    global image_data
    if msg.topic == RESP_TOPIC:
        image_data = msg.payload
        print(f"✅ Received image ({len(image_data)} bytes)")
        # mg.save("response_image.png")
        # print("Image saved as 'response_image.png'")

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_message = on_message
client.connect(BROKER, PORT)
client.subscribe(RESP_TOPIC)
client.loop_start()

# Send request
print("📡 Requesting image...")
client.publish(REQ_TOPIC, "get")

# Wait up to 5 seconds for response
timeout = 10
start = time.time()
while image_data is None and (time.time() - start) < timeout:
    time.sleep(0.1)

if image_data:
    try:
        img = Image.open(io.BytesIO(image_data))
        # img.show()
        # print("🖼️ Displayed!")
        img.save("response_image.png")
        print("Image saved as 'response_image.png'")
    except Exception as e:
        print(f"❌ Decode error: {e}")
else:
    print("⚠️ No image received.")

client.loop_stop()
client.disconnect()