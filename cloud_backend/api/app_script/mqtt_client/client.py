import paho.mqtt.client as mqtt
import ssl
import uuid
from config import (
    MQTT_BROKER,
    MQTT_PORT,
    MQTT_USER,
    MQTT_PASSWORD,
    MQTT_TOPIC_ALERT,
    MQTT_TOPIC_SENSOR,
)
from utils.terminal import print_info, print_error, print_success, print_warning


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print_success(f"Connected to MQTT broker successfully")
        # Subscribe to wildfire alerts topic
        client.subscribe(MQTT_TOPIC_ALERT)
        print_info(f"Subscribed to {MQTT_TOPIC_ALERT}")
        # Subscribe to sensor data topic
        client.subscribe(MQTT_TOPIC_SENSOR)
        print_info(f"Subscribed to {MQTT_TOPIC_SENSOR}")
    else:
        print_error(f"Failed to connect to MQTT broker with result code {rc}")


def on_connect_fail(client, userdata):
    print_error(f"Connection failed: Check broker address and credentials")


def setup_mqtt(message_callback):
    """Setup MQTT client with the specified message callback"""
    # Create a client with a unique ID to avoid broker connection conflicts
    client_id = f"python-mqtt-{uuid.uuid4().hex}"
    client = mqtt.Client(client_id=client_id, protocol=mqtt.MQTTv311, transport="tcp")

    # Set username and password
    client.username_pw_set(MQTT_USER, MQTT_PASSWORD)

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
    client.on_message = message_callback
    client.on_connect_fail = on_connect_fail

    # Connect to broker
    print_info(f"Connecting to MQTT broker at {MQTT_BROKER}:{MQTT_PORT}")
    try:
        client.connect(MQTT_BROKER, MQTT_PORT, 60)
        print_info("Connection attempt initiated")
    except Exception as e:
        print_error(f"Connection error: {e}")

    return client
