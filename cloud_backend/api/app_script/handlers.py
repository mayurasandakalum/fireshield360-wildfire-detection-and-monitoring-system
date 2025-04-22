import json
import threading
import time
from datetime import datetime

from .camera.capture import capture_image
from .storage.image_storage import create_alert_folder, save_image
from .config import CAPTURE_DURATION_SECONDS, CAPTURE_INTERVAL_SECONDS


def capture_images_for_duration(
    alert_folder,
    payload,
    duration_seconds=CAPTURE_DURATION_SECONDS,
    interval_seconds=CAPTURE_INTERVAL_SECONDS,
):
    """Capture multiple images over a specified duration and save them to the alert folder"""
    end_time = time.time() + duration_seconds
    image_count = 0

    while time.time() < end_time:
        print(f"Capturing image {image_count+1}...")
        image_data = capture_image()

        if image_data:
            save_image(image_data, alert_folder, image_count, payload)
            image_count += 1

        # Sleep for the interval time, but don't exceed the end time
        remaining_time = end_time - time.time()
        if remaining_time <= 0:
            break
        sleep_time = min(interval_seconds, remaining_time)
        time.sleep(sleep_time)


def on_message(client, userdata, msg):
    """Handler for MQTT messages"""
    try:
        # Parse the MQTT message
        payload = json.loads(msg.payload.decode())

        if msg.topic == "esp32_01/wildfire/alert" and payload.get(
            "wildfire_detected", False
        ):
            print(f"Wildfire alert received: {payload}")

            # Get timestamp from the payload or use current time
            timestamp = payload.get(
                "timestamp", datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
            )

            # Create a folder for this specific alert based on timestamp
            alert_folder = create_alert_folder(timestamp)

            # Start capturing images in a separate thread so it doesn't block the MQTT client
            image_thread = threading.Thread(
                target=capture_images_for_duration,
                args=(alert_folder, payload),
                kwargs={
                    "duration_seconds": CAPTURE_DURATION_SECONDS,
                    "interval_seconds": CAPTURE_INTERVAL_SECONDS,
                },
            )
            image_thread.daemon = True
            image_thread.start()
            print(
                f"Started image capture process for {CAPTURE_DURATION_SECONDS} seconds"
            )

    except Exception as e:
        print(f"Error processing MQTT message: {e}")
