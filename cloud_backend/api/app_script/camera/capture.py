import requests
import time
from datetime import datetime
from config import CAPTURE_URL
from utils.terminal import print_error, print_warning


def capture_image(timeout=10):
    """Capture a single image from the ESP32 camera"""
    try:
        response = requests.get(CAPTURE_URL, timeout=timeout)
        if response.status_code == 200:
            return response.content
        else:
            print_warning(
                f"Failed to capture image. Status code: {response.status_code}"
            )
            return None
    except Exception as e:
        print_error(f"Error capturing image: {e}")
        return None
