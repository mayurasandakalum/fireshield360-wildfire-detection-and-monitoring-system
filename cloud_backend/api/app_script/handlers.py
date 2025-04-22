import json
import threading
import time
import os
import traceback
from datetime import datetime

from camera.capture import capture_image, capture_image_alternative
from storage.image_storage import create_alert_folder, save_image
from config import (
    CAPTURE_DURATION_SECONDS,
    CAPTURE_INTERVAL_SECONDS,
    AI_MODELS_DIR,
    AI_MODEL_PATH,
    AI_CONF_THRESHOLD,
    AI_IOU_THRESHOLD,
    FRAMESIZE_UXGA,
)
from ai_detection.fire_detector import FireDetector
from utils.terminal import (
    print_info,
    print_warning,
    print_error,
    print_alert,
    print_separator,
    print_success,
    print_completion_banner,
)

# Create AI models directory
os.makedirs(AI_MODELS_DIR, exist_ok=True)

# Create FireDetector instance
fire_detector = FireDetector(
    model_path=AI_MODEL_PATH,
    conf_threshold=AI_CONF_THRESHOLD,
    iou_threshold=AI_IOU_THRESHOLD,
)

# Track active capture sessions
active_captures = 0
capture_lock = threading.Lock()


def capture_images_for_duration(
    alert_folder,
    payload,
    duration_seconds=CAPTURE_DURATION_SECONDS,
    interval_seconds=CAPTURE_INTERVAL_SECONDS,
):
    """Capture multiple images over a specified duration and save them to the alert folder"""
    global active_captures

    # Increment active captures counter
    with capture_lock:
        active_captures += 1

    try:
        end_time = time.time() + duration_seconds
        image_count = 0
        alert_id = os.path.basename(alert_folder)

        # Check if model file exists
        if not os.path.exists(AI_MODEL_PATH):
            print_warning(f"YOLO model not found at {AI_MODEL_PATH}")
            print_warning(
                f"Please download a YOLOv8 model and place it in the {AI_MODELS_DIR} directory"
            )
            print_warning("Fire detection will be skipped")
            use_ai = False
        else:
            # Preload the model
            try:
                if not fire_detector.load_model():
                    print_warning("Failed to load YOLO model")
                    print_warning("Fire detection will be skipped")
                    use_ai = False
                else:
                    use_ai = True
                    print_success("YOLO model loaded successfully for fire detection")
            except Exception as e:
                print_error(f"Error loading model: {e}")
                use_ai = False
                print_warning("Fire detection will be skipped due to error")

        while time.time() < end_time:
            try:
                print_info(f"Capturing image {image_count+1}...")

                # First try with the alternative method for better reliability
                image_data = capture_image_alternative(framesize=FRAMESIZE_UXGA)

                if image_data:
                    # Process with AI if model is available
                    detection_result = None
                    if use_ai:
                        try:
                            print_info(
                                "Processing image with YOLO fire detection model..."
                            )
                            # Use a try-except block with timeout for AI detection
                            try:
                                (
                                    result_img,
                                    detection_text,
                                    fire_detected,
                                    detection_info,
                                ) = fire_detector.detect(image_data)

                                if result_img is not None:
                                    print_info(f"Detection result: {detection_text}")
                                    if fire_detected:
                                        print_alert("FIRE/SMOKE DETECTED IN IMAGE!")

                                    detection_result = (
                                        result_img,
                                        detection_text,
                                        fire_detected,
                                        detection_info,
                                    )
                            except Exception as ai_error:
                                print_error(f"AI detection failed: {ai_error}")
                                # Continue without AI detection
                        except Exception as outer_e:
                            print_error(f"Error during image detection: {outer_e}")
                            traceback.print_exc()
                            # Continue without AI detection

                    # Save images (original and detection if available)
                    try:
                        original_path, detection_path, metadata_path = save_image(
                            image_data,
                            alert_folder,
                            image_count,
                            payload,
                            detection_result,
                        )

                        # Save detection image if available
                        if detection_result and detection_result[0] is not None:
                            try:
                                fire_detector.save_detection_image(
                                    detection_result[0], detection_path
                                )
                                print_success(
                                    f"Detection image saved to {detection_path}"
                                )
                            except Exception as save_error:
                                print_error(
                                    f"Error saving detection image: {save_error}"
                                )
                    except Exception as e:
                        print_error(f"Error saving image: {e}")
                        traceback.print_exc()

                    image_count += 1
                else:
                    print_warning("Failed to capture image, skipping this interval")

            except Exception as e:
                print_error(f"Error in capture loop: {e}")
                traceback.print_exc()

            # Explicitly call garbage collection to free memory
            try:
                import gc

                gc.collect()
            except:
                pass

            # Sleep for the interval time, but don't exceed the end time
            remaining_time = end_time - time.time()
            if remaining_time <= 0:
                break
            sleep_time = min(interval_seconds, remaining_time)
            time.sleep(sleep_time)

        # Show completion banner
        completion_message = (
            f"CAPTURE COMPLETE: {image_count} images captured for alert {alert_id}"
        )
        print_completion_banner(completion_message)
        print_info(f"The system remains active and listening for new alerts")
        print_separator()

    except Exception as e:
        print_error(f"Fatal error in image capture thread: {e}")
        traceback.print_exc()
    finally:
        # Decrement active captures counter
        with capture_lock:
            active_captures -= 1


def on_message(client, userdata, msg):
    """Handler for MQTT messages"""
    try:
        # Parse the MQTT message
        payload = json.loads(msg.payload.decode())

        if msg.topic == "esp32_01/wildfire/alert" and payload.get(
            "wildfire_detected", False
        ):
            print_alert(f"Wildfire alert received: {payload}")
            print_separator()

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
                name=f"Capture-{os.path.basename(alert_folder)}",
            )
            # do not set daemon=True so we can wait on shutdown
            image_thread.start()
            print_info(
                f"Started image capture process for {CAPTURE_DURATION_SECONDS} seconds"
            )

    except Exception as e:
        print_error(f"Error processing MQTT message: {e}")
        traceback.print_exc()


def get_active_captures():
    """Get the number of active capture sessions"""
    with capture_lock:
        return active_captures
