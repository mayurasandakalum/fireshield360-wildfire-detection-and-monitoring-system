import paho.mqtt.client as mqtt
import requests
import os
import time
import json
import uuid
from datetime import datetime
import ssl
import threading

# MQTT broker settings
mqtt_broker = "7ce36aef28e949f4b384e4808389cffc.s1.eu.hivemq.cloud"
mqtt_port = 8883
mqtt_user = "hivemq.webclient.1745152369573"
mqtt_password = "f%h3.X!9WGm0HtoJ1sO&"

# ESP32 CAM settings
camera_ip = "192.168.8.194"  # Default IP for ESP32-CAM when using AI-THINKER model
capture_url = f"http://{camera_ip}/capture"

# Output folder for images
image_folder = os.path.join(
    os.path.dirname(os.path.abspath(__file__)), "wildfire_images"
)
os.makedirs(image_folder, exist_ok=True)


# Function to capture multiple images over a 1-minute period
def capture_images_for_duration(
    alert_folder, payload, duration_seconds=60, interval_seconds=5
):
    """Capture multiple images over a specified duration and save them to the alert folder"""
    end_time = time.time() + duration_seconds
    image_count = 0

    while time.time() < end_time:
        try:
            print(f"Capturing image {image_count+1}...")
            response = requests.get(capture_url, timeout=10)
            if response.status_code == 200:
                # Create filename with timestamp and image number
                current_time = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
                filename = f"{alert_folder}/img_{image_count:03d}_{current_time}_T{payload.get('temperature', 0)}_H{payload.get('humidity', 0)}_S{payload.get('smoke', 0)}.jpg"

                # Save the image
                with open(filename, "wb") as f:
                    f.write(response.content)

                print(f"Image saved to {filename}")
                image_count += 1
            else:
                print(f"Failed to capture image. Status code: {response.status_code}")
        except Exception as e:
            print(f"Error capturing image: {e}")

        # Sleep for the interval time, but don't exceed the end time
        remaining_time = end_time - time.time()
        if remaining_time <= 0:
            break
        sleep_time = min(interval_seconds, remaining_time)
        time.sleep(sleep_time)


# MQTT callback when a message is received
def on_message(client, userdata, msg):
    try:
        # Parse the MQTT message
        payload = json.loads(msg.payload.decode())

        if msg.topic == "esp32_01/sensors/data" and payload.get(
            "potential_wildfire", False
        ):
            print(f"Potential wildfire alert received: {payload}")

            # Get timestamp from the payload or use current time
            timestamp = payload.get(
                "timestamp", datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
            )
            safe_timestamp = timestamp.replace(":", "-").replace(" ", "_")

            # Create a folder for this specific alert based on timestamp
            alert_folder = os.path.join(image_folder, f"alert_{safe_timestamp}")
            os.makedirs(alert_folder, exist_ok=True)
            print(f"Created alert folder: {alert_folder}")

            # Start capturing images in a separate thread so it doesn't block the MQTT client
            image_thread = threading.Thread(
                target=capture_images_for_duration,
                args=(alert_folder, payload),
                kwargs={"duration_seconds": 60, "interval_seconds": 5},
            )
            image_thread.daemon = True
            image_thread.start()
            print(f"Started image capture process for 1 minute")

    except Exception as e:
        print(f"Error processing MQTT message: {e}")


def on_connect(client, userdata, flags, rc):
    print(f"Connected to MQTT broker with result code {rc}")
    # Subscribe to sensor data topic
    client.subscribe("esp32_01/sensors/data")
    print("Subscribed to esp32_01/sensors/data")


# MQTT client setup
def setup_mqtt():
    # Create a client with a unique ID to avoid broker connection conflicts
    client_id = f"python-mqtt-{uuid.uuid4().hex}"
    client = mqtt.Client(client_id=client_id, protocol=mqtt.MQTTv311, transport="tcp")

    # Set username and password
    client.username_pw_set(mqtt_user, mqtt_password)

    # Configure TLS with appropriate settings
    client.tls_set(
        certfile=None,
        keyfile=None,
        cert_reqs=ssl.CERT_REQUIRED,
        tls_version=ssl.PROTOCOL_TLS,
        ciphers=None,
    )

    # For testing: disable certificate verification if needed
    # Remove this in production when using proper certificates
    client.tls_insecure_set(True)

    # Set callbacks
    client.on_connect = on_connect
    client.on_message = on_message

    # Add connection error handling
    def on_connect_fail(client, userdata):
        print(f"Connection failed: Check broker address and credentials")

    client.on_connect_fail = on_connect_fail

    # Connect to broker
    print(f"Connecting to MQTT broker at {mqtt_broker}:{mqtt_port}")
    try:
        client.connect(mqtt_broker, mqtt_port, 60)
        print("Connection attempt initiated")
    except Exception as e:
        print(f"Connection error: {e}")
        # Return client anyway so we can still try loop_start() which will retry connection

    return client


def main():
    print("AI pipeline started")
    print(f"Images will be saved to: {image_folder}")

    # Setup MQTT client
    mqtt_client = setup_mqtt()

    # Start the MQTT loop
    mqtt_client.loop_start()

    try:
        print("Listening for wildfire alerts... Press Ctrl+C to exit")
        # Keep the program running
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("Shutting down...")
    finally:
        mqtt_client.loop_stop()
        mqtt_client.disconnect()


if __name__ == "__main__":
    main()
