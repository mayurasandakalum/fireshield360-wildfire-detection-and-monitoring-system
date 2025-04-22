import time
import os
from .mqtt_client.client import setup_mqtt
from .handlers import on_message
from .config import IMAGE_FOLDER


def main():
    """Main entry point for the AI pipeline"""
    print("AI pipeline started")
    print(f"Images will be saved to: {IMAGE_FOLDER}")

    # Ensure image directory exists
    os.makedirs(IMAGE_FOLDER, exist_ok=True)

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
