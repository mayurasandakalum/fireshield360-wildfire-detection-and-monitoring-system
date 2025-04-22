import requests
import time
import io
from datetime import datetime
import numpy as np
import cv2
from PIL import Image
from config import CAPTURE_URL, CONTROL_URL, DEFAULT_FRAMESIZE, FRAMESIZE_UXGA
from utils.terminal import print_info, print_error, print_warning, print_success


def set_camera_framesize(framesize=DEFAULT_FRAMESIZE, timeout=5, retries=2):
    """
    Set the camera framesize

    Args:
        framesize: Frame size enum value (default: UXGA 1600x1200)
        timeout: Request timeout in seconds
        retries: Number of retry attempts

    Returns:
        bool: True if successful, False otherwise
    """
    # Map framesize to readable resolution for logging
    framesize_map = {
        10: "UXGA (1600x1200)",
        9: "SXGA (1280x1024)",
        8: "XGA (1024x768)",
        7: "SVGA (800x600)",
        6: "VGA (640x480)",
        5: "CIF (400x296)",
        4: "QVGA (320x240)",
        3: "HQVGA (240x176)",
        0: "QQVGA (160x120)",
    }

    resolution_name = framesize_map.get(framesize, f"Unknown ({framesize})")
    control_url = f"{CONTROL_URL}?var=framesize&val={framesize}"

    for attempt in range(retries):
        try:
            print_info(f"Setting camera resolution to {resolution_name}...")
            response = requests.get(control_url, timeout=timeout)

            if response.status_code == 200:
                print_success(f"Camera resolution set to {resolution_name}")
                # Important: Add a delay to allow the camera to adjust
                delay = 1.5  # seconds
                print_info(f"Waiting {delay} seconds for camera to adjust...")
                time.sleep(delay)
                return True
            else:
                print_warning(
                    f"Failed to set camera resolution. Status code: {response.status_code}"
                )
                if attempt < retries - 1:
                    print_warning(f"Retrying...")
                    time.sleep(1)
        except Exception as e:
            print_error(f"Error setting camera resolution: {e}")
            if attempt < retries - 1:
                print_warning(f"Retrying...")
                time.sleep(1)

    return False


def capture_image(framesize=DEFAULT_FRAMESIZE, timeout=10, retries=3, retry_delay=2):
    """
    Capture a single image from the ESP32 camera with retries

    Args:
        framesize: Frame size enum value (default: UXGA 1600x1200)
        timeout: Request timeout in seconds
        retries: Number of retry attempts
        retry_delay: Delay between retries in seconds

    Returns:
        Image data or None if failed
    """
    # First set the camera framesize to UXGA (1600x1200)
    set_success = set_camera_framesize(framesize)
    if not set_success:
        print_warning(
            "Failed to set camera resolution, but will still attempt to capture"
        )

    # Then capture the image
    for attempt in range(retries):
        try:
            print_info(f"Capturing image...")
            response = requests.get(CAPTURE_URL, timeout=timeout)

            if response.status_code == 200:
                # Validate the image size
                try:
                    image_data = response.content
                    img = Image.open(io.BytesIO(image_data))
                    width, height = img.size

                    if width == 1600 and height == 1200:
                        print_success(
                            f"Image captured successfully at full UXGA resolution (1600x1200)"
                        )
                    else:
                        print_warning(
                            f"Image captured at {width}x{height} instead of 1600x1200"
                        )

                        # If resolution is wrong, try setting it again and recapturing
                        if attempt < retries - 1 and (width != 1600 or height != 1200):
                            print_warning(
                                "Wrong resolution! Setting resolution again..."
                            )
                            # Try more forceful resolution setting
                            for i in range(2):  # Try twice
                                set_camera_framesize(framesize)
                                time.sleep(1.5)  # Wait longer for camera to adjust
                            continue  # Skip to next capture attempt

                    return image_data
                except Exception as e:
                    print_warning(f"Could not verify image dimensions: {e}")
                    return response.content
            else:
                print_warning(
                    f"Failed to capture image. Status code: {response.status_code}"
                )
                if attempt < retries - 1:
                    print_warning(
                        f"Retrying in {retry_delay} seconds... (Attempt {attempt+1}/{retries})"
                    )
                    time.sleep(retry_delay)
        except requests.exceptions.ConnectionError as e:
            print_error(f"Connection error to camera: {e}")
            if attempt < retries - 1:
                print_warning(
                    f"Retrying in {retry_delay} seconds... (Attempt {attempt+1}/{retries})"
                )
                time.sleep(retry_delay)
        except requests.exceptions.Timeout as e:
            print_error(f"Timeout error capturing image: {e}")
            if attempt < retries - 1:
                print_warning(
                    f"Retrying in {retry_delay} seconds... (Attempt {attempt+1}/{retries})"
                )
                time.sleep(retry_delay)
        except Exception as e:
            print_error(f"Error capturing image: {e}")
            if attempt < retries - 1:
                print_warning(
                    f"Retrying in {retry_delay} seconds... (Attempt {attempt+1}/{retries})"
                )
                time.sleep(retry_delay)

    # If we reach here, all retries failed
    print_error("All capture attempts failed after multiple retries")
    return None


def capture_image_alternative(
    framesize=DEFAULT_FRAMESIZE, timeout=10, retries=3, retry_delay=2
):
    """
    Alternative capture method trying multiple approaches

    Args:
        framesize: Frame size enum value (default: UXGA 1600x1200)
        timeout: Request timeout in seconds
        retries: Number of retry attempts
        retry_delay: Delay between retries in seconds

    Returns:
        Image data or None if failed
    """
    # Try direct capture URL with framesize parameter
    capture_url_with_size = f"{CAPTURE_URL}?size={framesize}"

    for attempt in range(retries):
        try:
            print_info(f"Attempting alternative capture with size parameter...")
            response = requests.get(capture_url_with_size, timeout=timeout)

            if response.status_code == 200:
                # Validate image size
                try:
                    image_data = response.content
                    img = Image.open(io.BytesIO(image_data))
                    width, height = img.size
                    print_success(
                        f"Alternative capture successful, image size: {width}x{height}"
                    )
                    return image_data
                except Exception:
                    return response.content
            else:
                print_warning(
                    f"Alternative capture failed with status: {response.status_code}"
                )
        except Exception as e:
            print_error(f"Error in alternative capture: {e}")

    # If alternative method fails, fall back to regular method
    return capture_image(framesize, timeout, retries, retry_delay)
