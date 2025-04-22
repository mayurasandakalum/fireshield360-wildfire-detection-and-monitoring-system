import time
import os
from mqtt_client.client import setup_mqtt
from handlers import on_message
from config import IMAGE_FOLDER, AI_MODELS_DIR, AI_MODEL_PATH


def main():
    """Main entry point for the AI pipeline"""
    print("AI pipeline with YOLO Fire Detection started")
    print(f"Images will be saved to: {IMAGE_FOLDER}")

    # Ensure directories exist
    os.makedirs(IMAGE_FOLDER, exist_ok=True)
    os.makedirs(AI_MODELS_DIR, exist_ok=True)

    # Check if YOLO model exists
    if not os.path.exists(AI_MODEL_PATH):
        print(f"YOLO model not found at {AI_MODEL_PATH}")
        print(
            f"Please download a YOLOv8 model (e.g., yolov8m.pt) and place it in {AI_MODELS_DIR}"
        )
        print("The system will run without fire detection until the model is available")
    else:
        print(f"YOLO model found at {AI_MODEL_PATH}")

    # Setup MQTT client with our message handler
    mqtt_client = setup_mqtt(on_message)

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
