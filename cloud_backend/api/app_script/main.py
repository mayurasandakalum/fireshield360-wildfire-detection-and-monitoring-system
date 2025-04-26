import time
import os
import signal
import sys
import threading
from mqtt_client.client import setup_mqtt
from handlers import on_message, get_active_captures
from config import IMAGE_FOLDER, AI_MODELS_DIR, AI_MODEL_PATH
from utils.terminal import (
    print_header,
    print_info,
    print_warning,
    print_error,
    print_separator,
    print_success,
)

# Flag to control the main loop
running = True


# Heartbeat thread to monitor the application
def heartbeat():
    heartbeat_interval = 30  # seconds
    while running:
        active_sessions = get_active_captures()
        status_msg = "Application heartbeat - system is running"
        if active_sessions > 0:
            status_msg += f" with {active_sessions} active capture session(s)"
        print_info(status_msg)
        time.sleep(heartbeat_interval)


# Signal handler for graceful shutdown
def signal_handler(sig, frame):
    global running
    print_warning("Shutdown signal received, exiting gracefully...")
    running = False


def main():
    """Main entry point for the AI pipeline"""
    global running

    # Register signal handlers for graceful shutdown
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    print_header("AI PIPELINE WITH YOLO FIRE DETECTION")
    print_info(f"Images will be saved to: {IMAGE_FOLDER}")
    print_separator()

    # Ensure directories exist
    os.makedirs(IMAGE_FOLDER, exist_ok=True)
    os.makedirs(AI_MODELS_DIR, exist_ok=True)

    # Check if YOLO model exists
    if not os.path.exists(AI_MODEL_PATH):
        print_warning(f"YOLO model not found at {AI_MODEL_PATH}")
        print_warning(
            f"Please download a YOLOv8 model (e.g., yolov8m.pt) and place it in {AI_MODELS_DIR}"
        )
        print_warning(
            "The system will run without fire detection until the model is available"
        )
    else:
        print_info(f"YOLO model found at {AI_MODEL_PATH}")

    # Setup MQTT client with our message handler
    mqtt_client = setup_mqtt(on_message)

    # Start the MQTT loop
    mqtt_client.loop_start()
    print_separator()

    # Start a heartbeat thread to monitor the application
    heartbeat_thread = threading.Thread(target=heartbeat, name="Heartbeat")
    heartbeat_thread.daemon = True
    heartbeat_thread.start()

    try:
        print_info("Listening for wildfire alerts... Press Ctrl+C to exit")
        # Keep the program running until the running flag is False
        while running:
            time.sleep(1)
        print_info("Main loop exited")
        # wait for any ongoing capture threads to complete
        remaining = get_active_captures()
        if remaining > 0:
            print_info(
                f"Waiting for {remaining} active capture session(s) to finish..."
            )
        while get_active_captures() > 0:
            time.sleep(1)
        print_info("All capture sessions have completed")
    except Exception as e:
        print_error(f"Error in main loop: {e}")
        import traceback

        traceback.print_exc()
    finally:
        print_info("Shutting down...")
        # Explicitly stop and disconnect MQTT client
        try:
            mqtt_client.loop_stop()
            mqtt_client.disconnect()
            print_success("MQTT client stopped successfully")
        except Exception as e:
            print_error(f"Error stopping MQTT client: {e}")

        print_success("Shutdown complete")


if __name__ == "__main__":
    main()
