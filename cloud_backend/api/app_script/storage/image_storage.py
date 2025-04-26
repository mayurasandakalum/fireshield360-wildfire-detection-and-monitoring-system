import os
import time
import json
from datetime import datetime
from config import IMAGE_FOLDER, CAPTURE_DURATION_SECONDS, CAPTURE_INTERVAL_SECONDS
from utils.terminal import print_info, print_error, print_success

# Ensure the base image folder exists
os.makedirs(IMAGE_FOLDER, exist_ok=True)


def create_alert_folder(timestamp=None):
    """Create a folder for a specific alert based on timestamp"""
    if timestamp is None:
        timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")

    safe_timestamp = timestamp.replace(":", "-").replace(" ", "_")
    alert_folder = os.path.join(IMAGE_FOLDER, f"alert_{safe_timestamp}")
    os.makedirs(alert_folder, exist_ok=True)
    print_info(f"Created alert folder: {alert_folder}")
    return alert_folder


def save_image(
    image_data, folder_path, image_count, sensor_data, detection_result=None
):
    """
    Save image data to the specified folder with metadata in the filename

    Args:
        image_data: Raw image binary data
        folder_path: Path to save images
        image_count: Image sequence number
        sensor_data: Dictionary containing sensor readings
        detection_result: Optional AI detection results

    Returns:
        tuple: (original_path, detection_path, metadata_path)
    """
    if image_data is None:
        return None, None, None

    current_time = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    base_filename = (
        f"img_{image_count:03d}_{current_time}_"
        f"T{sensor_data.get('temperature', 0)}_"
        f"H{sensor_data.get('humidity', 0)}_"
        f"S{sensor_data.get('smoke', 0)}"
    )

    # Original image path
    original_path = f"{folder_path}/{base_filename}_original.jpg"

    # Detection image path (if available)
    detection_path = (
        f"{folder_path}/{base_filename}_detection.jpg" if detection_result else None
    )

    # Metadata path for saving detection results
    metadata_path = f"{folder_path}/{base_filename}_metadata.json"

    try:
        # Save original image
        with open(original_path, "wb") as f:
            f.write(image_data)
        print_success(f"Original image saved to {original_path}")

        # Save detection metadata if available
        if detection_result:
            detection_img, detection_text, fire_detected, detection_info = (
                detection_result
            )

            # Save detection info
            metadata = {
                "timestamp": current_time,
                "detection_text": detection_text,
                "fire_detected": fire_detected,
                "sensor_data": sensor_data,
                "detection_info": detection_info,
            }

            with open(metadata_path, "w") as f:
                json.dump(metadata, f, indent=4)

            # Return file paths
            return original_path, detection_path, metadata_path

        return original_path, None, None

    except Exception as e:
        print_error(f"Error saving image: {e}")
        return None, None, None
