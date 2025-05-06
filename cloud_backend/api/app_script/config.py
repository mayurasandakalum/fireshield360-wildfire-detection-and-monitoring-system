import os

# MQTT broker settings
MQTT_BROKER = "YOUR_MQTT_BROKER_ADDRESS"  # Replace with your MQTT broker address
MQTT_PORT = 8883
MQTT_USER = "YOUR_MQTT_USERNAME"  # Replace with your MQTT username
MQTT_PASSWORD = "YOUR_MQTT_PASSWORD"  # Replace with your MQTT password
MQTT_TOPIC_SENSOR = "esp32_01/sensors/data"
MQTT_TOPIC_VERIFIED = "esp32_01/verified/status"

# ESP32 CAM settings
CAMERA_IP = "192.168.1.100"  # Replace with your ESP32 camera's IP address
CAPTURE_URL = f"http://{CAMERA_IP}/capture"
CONTROL_URL = f"http://{CAMERA_IP}/control"

# ESP32 Camera framesize constants
FRAMESIZE_UXGA = 10  # 1600x1200
FRAMESIZE_SXGA = 9  # 1280x1024
FRAMESIZE_XGA = 8  # 1024x768
FRAMESIZE_SVGA = 7  # 800x600
FRAMESIZE_VGA = 6  # 640x480
FRAMESIZE_CIF = 5  # 400x296
FRAMESIZE_QVGA = 4  # 320x240
FRAMESIZE_HQVGA = 3  # 240x176
FRAMESIZE_QQVGA = 0  # 160x120

# Default framesize
DEFAULT_FRAMESIZE = FRAMESIZE_UXGA

# Image settings
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
IMAGE_FOLDER = os.path.join(BASE_DIR, "wildfire_images")

# Capture settings
CAPTURE_DURATION_SECONDS = 60
CAPTURE_INTERVAL_SECONDS = 5

# AI Detection settings
AI_MODELS_DIR = os.path.join(BASE_DIR, "fire_models")
AI_MODEL_PATH = os.path.join(AI_MODELS_DIR, "fire_l.pt")
AI_CONF_THRESHOLD = 0.10
AI_IOU_THRESHOLD = 0.10

# Telegram notification settings
TELEGRAM_TOKEN = "YOUR_TELEGRAM_BOT_TOKEN"  # Replace with your Telegram bot token
TELEGRAM_CHAT_ID = "YOUR_TELEGRAM_CHAT_ID"  # Replace with your Telegram chat ID
TELEGRAM_ENABLED = True  # Enable/disable Telegram notifications
