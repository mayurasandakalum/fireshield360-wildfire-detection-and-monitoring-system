import os

# MQTT broker settings
MQTT_BROKER = "7ce36aef28e949f4b384e4808389cffc.s1.eu.hivemq.cloud"
MQTT_PORT = 8883
MQTT_USER = "hivemq.webclient.1745152369573"
MQTT_PASSWORD = "f%h3.X!9WGm0HtoJ1sO&"
MQTT_TOPIC_ALERT = "esp32_01/wildfire/alert"

# ESP32 CAM settings
CAMERA_IP = "192.168.8.194"
CAPTURE_URL = f"http://{CAMERA_IP}/capture"

# Image settings
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
IMAGE_FOLDER = os.path.join(BASE_DIR, "wildfire_images")

# Capture settings
CAPTURE_DURATION_SECONDS = 60
CAPTURE_INTERVAL_SECONDS = 5

# AI Detection settings
AI_MODELS_DIR = os.path.join(BASE_DIR, "fire_models")
AI_MODEL_PATH = os.path.join(AI_MODELS_DIR, "yolov8l.pt")
AI_CONF_THRESHOLD = 0.25
AI_IOU_THRESHOLD = 0.45
