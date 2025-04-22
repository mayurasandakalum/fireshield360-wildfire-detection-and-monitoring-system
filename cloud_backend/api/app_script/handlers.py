import json
import threading
import time
import os
from datetime import datetime

from camera.capture import capture_image
from storage.image_storage import create_alert_folder, save_image
from config import (
    CAPTURE_DURATION_SECONDS,
    CAPTURE_INTERVAL_SECONDS,
    AI_MODELS_DIR,
    AI_MODEL_PATH,
    AI_CONF_THRESHOLD,
    AI_IOU_THRESHOLD,
)
from ai_detection.fire_detector import FireDetector

# Create AI models directory
os.makedirs(AI_MODELS_DIR, exist_ok=True)

# Create FireDetector instance
fire_detector = FireDetector(
    model_path=AI_MODEL_PATH,
    conf_threshold=AI_CONF_THRESHOLD,
    iou_threshold=AI_IOU_THRESHOLD,
)


def capture_images_for_duration(
    alert_folder,
    payload,
    duration_seconds=CAPTURE_DURATION_SECONDS,
    interval_seconds=CAPTURE_INTERVAL_SECONDS,
):
    """Capture multiple images over a specified duration and save them to the alert folder"""
    end_time = time.time() + duration_seconds
    image_count = 0

    # Check if model file exists
    if not os.path.exists(AI_MODEL_PATH):
        print(f"Warning: YOLO model not found at {AI_MODEL_PATH}")
        print(
            f"Please download a YOLOv8 model and place it in the {AI_MODELS_DIR} directory"
        )
        print("Fire detection will be skipped")
        use_ai = False
    else:
        # Preload the model
        if not fire_detector.load_model():
            print("Warning: Failed to load YOLO model")
            print("Fire detection will be skipped")
            use_ai = False
        else:
            use_ai = True
            print("YOLO model loaded successfully for fire detection")

    while time.time() < end_time:
        print(f"Capturing image {image_count+1}...")
        image_data = capture_image()

        if image_data:
            # Process with AI if model is available
            detection_result = None
            if use_ai:
                print("Processing image with YOLO fire detection model...")
                result_img, detection_text, fire_detected, detection_info = (
                    fire_detector.detect(image_data)
                )

                if result_img is not None:
                    print(f"Detection result: {detection_text}")
                    if fire_detected:
                        print("FIRE/SMOKE DETECTED IN IMAGE!")

                    detection_result = (
                        result_img,
                        detection_text,
                        fire_detected,
                        detection_info,
                    )

            # Save images (original and detection if available)
            original_path, detection_path, metadata_path = save_image(
                image_data, alert_folder, image_count, payload, detection_result
            )

            # Save detection image if available
            if detection_result and detection_result[0] is not None:
                fire_detector.save_detection_image(detection_result[0], detection_path)
                print(f"Detection image saved to {detection_path}")

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
