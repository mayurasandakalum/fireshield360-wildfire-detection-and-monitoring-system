import os
import sys
from config import TELEGRAM_TOKEN, TELEGRAM_CHAT_ID
from utils.terminal import print_info, print_error, print_success


def test_telegram():
    """Test the Telegram notification functionality"""
    print_info("Testing Telegram notifications...")

    try:
        from notifications.telegram_notifier import (
            initialize_telegram,
            send_message,
            send_photo,
        )

        # Initialize Telegram
        if initialize_telegram(TELEGRAM_TOKEN, TELEGRAM_CHAT_ID):
            print_success("Telegram initialization successful")
        else:
            print_error("Telegram initialization failed")
            return False

        # Send a test message
        if send_message("This is a test message from the Wildfire Detection System"):
            print_success("Test message sent successfully")
        else:
            print_error("Failed to send test message")
            return False

        # Find a sample image to send
        sample_image = None
        for root, dirs, files in os.walk("wildfire_images"):
            for file in files:
                if file.endswith((".jpg", ".jpeg", ".png")):
                    sample_image = os.path.join(root, file)
                    break
            if sample_image:
                break

        if not sample_image:
            print_info("No sample images found in wildfire_images directory")
            # Create a simple test image
            from PIL import Image, ImageDraw, ImageFont
            import numpy as np

            # Create a new image with a solid color
            img = Image.new("RGB", (400, 200), color=(73, 109, 137))
            d = ImageDraw.Draw(img)
            d.text(
                (10, 10),
                "Test Image\nFrom Wildfire Detection System",
                fill=(255, 255, 255),
            )

            # Save the image
            if not os.path.exists("wildfire_images"):
                os.makedirs("wildfire_images")
            sample_image = os.path.join("wildfire_images", "test_image.jpg")
            img.save(sample_image)
            print_info(f"Created test image at {sample_image}")

        # Send the sample image
        if send_photo(sample_image, "This is a test image"):
            print_success(f"Test image sent successfully from {sample_image}")
        else:
            print_error("Failed to send test image")
            return False

        return True
    except Exception as e:
        print_error(f"Error testing Telegram: {e}")
        import traceback

        traceback.print_exc()
        return False


if __name__ == "__main__":
    # Make sure we can import from parent directory
    sys.path.insert(0, os.path.abspath(os.path.dirname(__file__)))

    # Create notifications directory if it doesn't exist
    if not os.path.exists("notifications"):
        os.makedirs("notifications")
        with open("notifications/__init__.py", "w") as f:
            f.write("# This file makes the directory a Python package\n")

    if test_telegram():
        print_success("All Telegram tests passed!")
        sys.exit(0)
    else:
        print_error("Telegram tests failed!")
        sys.exit(1)
