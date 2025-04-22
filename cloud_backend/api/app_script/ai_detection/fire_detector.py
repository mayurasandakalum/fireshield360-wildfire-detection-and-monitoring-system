import cv2
import numpy as np
import os
from ultralytics import YOLO
from PIL import Image
import io
from utils.terminal import print_info, print_error, print_warning, print_success

# Set environment variable to avoid issues with OpenCV
os.environ["KMP_DUPLICATE_LIB_OK"] = "True"


class FireDetector:
    def __init__(self, model_path, conf_threshold=0.25, iou_threshold=0.45):
        self.model_path = model_path
        self.conf_threshold = conf_threshold
        self.iou_threshold = iou_threshold
        self.model = None

    def load_model(self):
        """Load the YOLO model"""
        try:
            print_info(f"Loading YOLO model from {self.model_path}")
            self.model = YOLO(self.model_path)
            print_success("YOLO model loaded successfully")
            return True
        except Exception as e:
            print_error(f"Error loading YOLO model: {e}")
            return False

    def detect(self, image_data):
        """
        Detect fire and smoke in image

        Args:
            image_data: Raw image bytes

        Returns:
            tuple: (processed_image, detection_text, fire_detected, detection_info)
        """
        if self.model is None:
            if not self.load_model():
                return None, "Failed to load model", False, {}

        try:
            # Convert image data to numpy array
            nparr = np.frombuffer(image_data, np.uint8)
            img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)

            # Predict objects using the model
            results = self.model.predict(
                img,
                conf=self.conf_threshold,
                iou=self.iou_threshold,
                device="cpu",
            )

            # Process results
            result = results[0]
            class_names = self.model.model.names
            classes = result.boxes.cls
            class_counts = {}

            # Check if fire or smoke is detected
            fire_detected = False

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
            latency = sum(result.speed.values())  # in ms
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

            return result_img, detection_text, fire_detected, detection_info

        except Exception as e:
            print_error(f"Error detecting fire/smoke: {e}")
            return None, f"Error: {str(e)}", False, {}

    def save_detection_image(self, detection_img, output_path):
        """Save the detection image to the specified path"""
        try:
            cv2.imwrite(output_path, detection_img)
            return output_path
        except Exception as e:
            print_error(f"Error saving detection image: {e}")
            return None
