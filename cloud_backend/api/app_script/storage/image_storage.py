import os
import time
from datetime import datetime
from ..config import IMAGE_FOLDER, CAPTURE_DURATION_SECONDS, CAPTURE_INTERVAL_SECONDS

# Ensure the base image folder exists
os.makedirs(IMAGE_FOLDER, exist_ok=True)


def create_alert_folder(timestamp=None):
    """Create a folder for a specific alert based on timestamp"""
    if timestamp is None:
        timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")

    safe_timestamp = timestamp.replace(":", "-").replace(" ", "_")
    alert_folder = os.path.join(IMAGE_FOLDER, f"alert_{safe_timestamp}")
    os.makedirs(alert_folder, exist_ok=True)
    print(f"Created alert folder: {alert_folder}")
    return alert_folder


def save_image(image_data, folder_path, image_count, sensor_data):
    """Save image data to the specified folder with metadata in the filename"""
    if image_data is None:
        return None

    current_time = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    filename = (
        f"{folder_path}/img_{image_count:03d}_{current_time}_"
        f"T{sensor_data.get('temperature', 0)}_"
        f"H{sensor_data.get('humidity', 0)}_"
        f"S{sensor_data.get('smoke', 0)}.jpg"
    )

    try:
        with open(filename, "wb") as f:
            f.write(image_data)
        print(f"Image saved to {filename}")
        return filename
    except Exception as e:
        print(f"Error saving image: {e}")
        return None
