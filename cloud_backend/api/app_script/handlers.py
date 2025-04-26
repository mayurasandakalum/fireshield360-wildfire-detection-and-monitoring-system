import json
import threading
import time
import os
import traceback
from datetime import datetime
import paho.mqtt.publish as mqtt_publish

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
    MQTT_BROKER,
    MQTT_PORT,
    MQTT_USER,
    MQTT_PASSWORD,
    MQTT_TOPIC_ALERT,
    MQTT_TOPIC_SENSOR,
    MQTT_TOPIC_VERIFIED,
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


# Function to publish verified wildfire status
def publish_verified_status(original_data, wildfire_detected):
    """Publish the verified wildfire status to the MQTT verified topic"""
    try:
        # Create a deep copy of the original data and add verified status
        verified_data = original_data.copy() if original_data else {}
        verified_data["wildfire_detected"] = wildfire_detected

        # Convert to JSON
        payload = json.dumps(verified_data)

        # Setup auth for MQTT
        auth = {"username": MQTT_USER, "password": MQTT_PASSWORD}

        # Setup TLS parameters correctly
        tls = {"tls_version": 2, "insecure": True}  # For testing only

        # Publish to the verified topic
        mqtt_publish.single(
            MQTT_TOPIC_VERIFIED,
            payload=payload,
            hostname=MQTT_BROKER,
            port=MQTT_PORT,
            auth=auth,
            tls=tls,
        )

        print_success(
            f"Published verified status (wildfire_detected={wildfire_detected}) to {MQTT_TOPIC_VERIFIED}"
        )
        return True
    except Exception as e:
        print_error(f"Error publishing verified status: {e}")
        traceback.print_exc()
        return False


# Function to verify potential wildfire with camera and AI
def verify_potential_wildfire(payload):
    """Verify if there's an actual wildfire by capturing multiple images over a minute and using AI detection"""
    try:
        print_info("Verifying potential wildfire with multiple images over 1 minute...")

        # Check if model file exists
        if not os.path.exists(AI_MODEL_PATH):
            print_warning(f"YOLO model not found at {AI_MODEL_PATH}")
            # If no model, assume sensor data is correct and forward without verification
            publish_verified_status(payload, payload.get("potential_wildfire", False))
            return

        # Create a folder for this verification
        timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        alert_folder = create_alert_folder(timestamp)

        # Initialize detection results tracking
        total_images = 0
        fire_detected_count = 0

        # Start capturing and processing images for 1 minute
        end_time = time.time() + 60  # 1 minute
        image_count = 0

        print_info(f"Starting 1-minute image capture and analysis sequence...")

        # Preload the model
        if not fire_detector.load_model():
            print_warning("Failed to load YOLO model")
            publish_verified_status(payload, payload.get("potential_wildfire", False))
            return

        while time.time() < end_time:
            try:
                print_info(f"Capturing image {image_count+1}...")

                # Capture image
                image_data = capture_image_alternative(framesize=FRAMESIZE_UXGA)

                if not image_data:
                    print_warning(
                        f"Failed to capture image {image_count+1}, skipping..."
                    )
                    # Skip this iteration but continue loop
                    continue

                # Process with YOLO detection
                try:
                    print_info(f"Processing image {image_count+1} with YOLO...")
                    result_img, detection_text, fire_detected, detection_info = (
                        fire_detector.detect(image_data)
                    )

                    total_images += 1
                    if fire_detected:
                        fire_detected_count += 1
                        print_alert(f"FIRE/SMOKE DETECTED in image {image_count+1}!")

                    # Save the image with detection results
                    detection_result = (
                        result_img,
                        detection_text,
                        fire_detected,
                        detection_info,
                    )
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
                            print_success(f"Detection image saved to {detection_path}")
                        except Exception as save_error:
                            print_error(f"Error saving detection image: {save_error}")

                except Exception as ai_error:
                    print_error(
                        f"AI detection failed for image {image_count+1}: {ai_error}"
                    )

                image_count += 1

                # Add a 5-second wait after capturing and processing each image
                remaining_time = end_time - time.time()
                if remaining_time <= 0:
                    break

                wait_time = min(5, remaining_time)  # Wait 5 seconds or until end time
                if wait_time > 0:
                    print_info(f"Waiting 5 seconds before next capture...")
                    time.sleep(wait_time)

                # Check if we've run out of time
                if time.time() >= end_time:
                    break

            except Exception as e:
                print_error(f"Error processing image {image_count+1}: {e}")
                traceback.print_exc()

        # Make final determination based on all images
        final_fire_detected = False
        detection_ratio = 0

        if total_images > 0:
            detection_ratio = fire_detected_count / total_images
            # If at least 20% of images detected fire, consider it a verified wildfire
            final_fire_detected = detection_ratio >= 0.2

            print_separator()
            print_info(f"Verification complete: Analyzed {total_images} images")
            print_info(
                f"Fire detected in {fire_detected_count} images ({detection_ratio*100:.1f}%)"
            )

            if final_fire_detected:
                print_alert("WILDFIRE CONFIRMED by image analysis!")
            else:
                print_info("No significant wildfire evidence found in images")
        else:
            print_warning("No valid images were captured for verification")
            # Default to sensor data if no images were processed
            final_fire_detected = payload.get("potential_wildfire", False)

        # Include additional data in the verified status
        payload_with_stats = payload.copy() if payload else {}
        payload_with_stats["verification_images"] = total_images
        payload_with_stats["fire_detected_images"] = fire_detected_count
        payload_with_stats["detection_ratio"] = detection_ratio

        # Publish the verified status
        publish_verified_status(payload_with_stats, final_fire_detected)

    except Exception as e:
        print_error(f"Error verifying potential wildfire: {e}")
        traceback.print_exc()
        # In case of error, forward the original potential_wildfire value
        publish_verified_status(payload, payload.get("potential_wildfire", False))


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
                    print_warning("Failed to capture image, skipping this iteration")

            except Exception as e:
                print_error(f"Error in capture loop: {e}")
                traceback.print_exc()

            # Explicitly call garbage collection to free memory
            try:
                import gc

                gc.collect()
            except:
                pass

            # Add a 5-second wait after capturing and processing each image
            remaining_time = end_time - time.time()
            if remaining_time <= 0:
                break

            wait_time = min(5, remaining_time)  # Wait 5 seconds or until end time
            if wait_time > 0:
                print_info(f"Waiting 5 seconds before next capture...")
                time.sleep(wait_time)

            # Check if we've run out of time
            if time.time() >= end_time:
                break

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

        # Process messages from the sensor data topic
        if msg.topic == MQTT_TOPIC_SENSOR:
            print_info(f"Sensor data received: {payload}")

            # Check if potential_wildfire flag is true
            if payload.get("potential_wildfire", False):
                print_alert("Potential wildfire detected by sensors!")
                print_separator()

                # Verify with camera and AI in a separate thread
                verify_thread = threading.Thread(
                    target=verify_potential_wildfire,
                    args=(payload,),
                    name=f"Verify-{datetime.now().strftime('%H%M%S')}",
                )
                verify_thread.start()
                print_info("Started wildfire verification process")
            else:
                # No potential wildfire, just forward the data
                print_info("No potential wildfire detected, forwarding data")
                publish_verified_status(payload, False)

        # Process legacy wildfire alert messages
        elif msg.topic == MQTT_TOPIC_ALERT and payload.get("potential_wildfire", False):
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
