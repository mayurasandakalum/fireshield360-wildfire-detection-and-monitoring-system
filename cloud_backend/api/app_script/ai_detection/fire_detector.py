import cv2
import numpy as np
import os
import time
import gc
import threading
from ultralytics import YOLO
from PIL import Image
import io
from utils.terminal import print_info, print_error, print_warning, print_success

# Set environment variable to avoid issues with OpenCV
os.environ["KMP_DUPLICATE_LIB_OK"] = "True"


class TimeoutError(Exception):
    pass


# Cross-platform timeout solution using threads
class ThreadWithTimeout(threading.Thread):
    def __init__(self, target=None, args=(), kwargs=None, timeout=None):
        threading.Thread.__init__(self, target=target, args=args, kwargs=kwargs or {})
        self.timeout = timeout
        self.result = None
        self.exception = None

    def run(self):
        try:
            if self._target:
                self.result = self._target(*self._args, **self._kwargs)
        except Exception as e:
            self.exception = e

    def join(self, timeout=None):
        super().join(timeout or self.timeout)
        if self.is_alive():
            raise TimeoutError(f"Operation timed out after {self.timeout} seconds")
        if self.exception:
            raise self.exception
        return self.result


class FireDetector:
    def __init__(self, model_path, conf_threshold=0.25, iou_threshold=0.45):
        self.model_path = model_path
        self.conf_threshold = conf_threshold
        self.iou_threshold = iou_threshold
        self.model = None
        self.model_loaded = False

    def _load_model_worker(self):
        """Worker function to load the model in a separate thread"""
        try:
            model = YOLO(self.model_path)
            # Immediately run a small inference to fully initialize the model
            dummy_img = np.zeros((100, 100, 3), dtype=np.uint8)
            model.predict(dummy_img, verbose=False)
            return model
        except Exception as e:
            print_error(f"Error in model loading worker: {e}")
            raise

    def load_model(self):
        """Load the YOLO model with cross-platform timeout"""
        try:
            if self.model_loaded and self.model is not None:
                print_info("YOLO model already loaded")
                return True

            print_info(f"Loading YOLO model from {self.model_path}")

            # Free up memory before loading model
            gc.collect()

            # Use threading-based timeout (30 seconds)
            try:
                load_thread = ThreadWithTimeout(
                    target=self._load_model_worker, timeout=30
                )
                load_thread.start()
                self.model = (
                    load_thread.join()
                )  # This will raise TimeoutError if it times out

            except TimeoutError:
                print_error("Model loading timed out after 30 seconds")
                return False
            except Exception as e:
                print_error(f"Error during model initialization: {e}")
                return False

            self.model_loaded = True
            print_success("YOLO model loaded successfully")
            return True
        except Exception as e:
            print_error(f"Error loading YOLO model: {e}")
            return False

    def _detect_worker(self, img):
        """Worker function to run detection in a separate thread"""
        return self.model.predict(
            img,
            conf=self.conf_threshold,
            iou=self.iou_threshold,
            device="cpu",
            verbose=False,
            max_det=50,
        )

    def detect(self, image_data):
        """
        Detect fire and smoke in image with cross-platform timeout

        Args:
            image_data: Raw image bytes

        Returns:
            tuple: (processed_image, detection_text, fire_detected, detection_info)
        """
        if self.model is None:
            if not self.load_model():
                return None, "Failed to load model", False, {}

        try:
            start_time = time.time()

            # Convert image data to numpy array
            nparr = np.frombuffer(image_data, np.uint8)
            img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)

            if img is None or img.size == 0:
                return None, "Invalid image data", False, {}

            # Use threading-based timeout for detection (15 seconds)
            try:
                detect_thread = ThreadWithTimeout(
                    target=self._detect_worker, args=(img,), timeout=15
                )
                detect_thread.start()
                results = (
                    detect_thread.join()
                )  # This will raise TimeoutError if it times out

            except TimeoutError:
                print_error("Detection timed out after 15 seconds, skipping")
                return None, "Detection timed out", False, {}
            except Exception as e:
                print_error(f"Error during detection: {e}")
                return None, f"Detection error: {str(e)}", False, {}

            # Process results
            if not results or len(results) == 0:
                return None, "No results from model", False, {}

            result = results[0]
            class_names = self.model.model.names

            # Check if fire or smoke is detected
            fire_detected = False
            class_counts = {}

            if hasattr(result, "boxes") and hasattr(result.boxes, "cls"):
                classes = result.boxes.cls

                # Count the number of occurrences for each class
                for c in classes:
                    c = int(c)
                    class_name = class_names[c]
                    class_counts[class_name] = class_counts.get(class_name, 0) + 1

                    # Check if fire or smoke is detected
                    if class_name.lower() in ["fire", "smoke"]:
                        fire_detected = True

            # Generate detection text
            detection_text = "Detected "
            for k, v in sorted(
                class_counts.items(), key=lambda item: item[1], reverse=True
            ):
                detection_text += f"{v} {k}"
                if v > 1:
                    detection_text += "s"
                detection_text += ", "

            detection_text = (
                detection_text[:-2] if class_counts else "No objects detected"
            )

            # Calculate inference latency
            end_time = time.time()
            latency = (end_time - start_time) * 1000  # in ms
            latency_sec = round(latency / 1000, 2)
            detection_text += f" in {latency_sec} seconds."

            # Get the annotated image
            result_img = result.plot()

            # Detection info dictionary
            detection_info = {
                "class_counts": class_counts,
                "fire_detected": fire_detected,
                "latency_ms": latency,
                "latency_sec": latency_sec,
            }

            # Clean up to free memory
            gc.collect()

            return result_img, detection_text, fire_detected, detection_info

        except Exception as e:
            print_error(f"Error detecting fire/smoke: {e}")
            return None, f"Error: {str(e)}", False, {}

    def save_detection_image(self, detection_img, output_path):
        """Save the detection image to the specified path"""
        try:
            # Ensure the output directory exists
            output_dir = os.path.dirname(output_path)
            os.makedirs(output_dir, exist_ok=True)

            # Make sure the image is valid
            if detection_img is None or detection_img.size == 0:
                print_error("Cannot save detection image: Image data is empty or None")
                return None

            # Save the image
            result = cv2.imwrite(output_path, detection_img)

            if not result:
                print_error(f"cv2.imwrite failed to save image to {output_path}")
                return None

            # Verify the file was created and has content
            if os.path.exists(output_path) and os.path.getsize(output_path) > 0:
                print_success(
                    f"Detection image saved successfully: {os.path.getsize(output_path)} bytes"
                )
                return output_path
            else:
                print_error(
                    f"Image file not created or empty after cv2.imwrite: {output_path}"
                )
                return None
        except Exception as e:
            print_error(f"Error saving detection image: {e}")
            import traceback

            traceback.print_exc()
            return None

    def __del__(self):
        """Cleanup when detector is destroyed"""
        try:
            # Free up resources
            self.model = None
            gc.collect()
        except:
            pass
