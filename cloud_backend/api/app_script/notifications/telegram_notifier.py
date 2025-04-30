import os
import requests
import time
from utils.terminal import print_info, print_error, print_success, print_warning

# Telegram configuration - will be updated from config
TELEGRAM_TOKEN = None
TELEGRAM_CHAT_ID = None

# Track alerts that have already been sent initial message
alert_initial_sent = set()


def initialize_telegram(token, chat_id):
    """Initialize the Telegram bot with the provided token and chat ID"""
    global TELEGRAM_TOKEN, TELEGRAM_CHAT_ID
    TELEGRAM_TOKEN = token
    TELEGRAM_CHAT_ID = chat_id
    print_info(f"Telegram bot initialized with chat ID: {TELEGRAM_CHAT_ID}")

    # Test the connection by sending a simple message
    result = send_message("Telegram bot initialized and ready to send fire alerts")
    if result:
        print_success("Telegram connection test successful")
    else:
        print_warning("Telegram connection test failed")

    return result


def send_message(message):
    """Send a text message to Telegram using the HTTP API directly"""
    if not TELEGRAM_TOKEN or not TELEGRAM_CHAT_ID:
        print_error("Telegram not initialized. Call initialize_telegram first.")
        return False

    try:
        url = f"https://api.telegram.org/bot{TELEGRAM_TOKEN}/sendMessage"
        params = {"chat_id": TELEGRAM_CHAT_ID, "text": message, "parse_mode": "HTML"}

        response = requests.post(url, params=params, timeout=10)

        if response.status_code == 200:
            print_success(f"Telegram message sent: {message[:50]}...")
            return True
        else:
            print_error(
                f"Failed to send message. Status code: {response.status_code}, Response: {response.text}"
            )
            return False
    except Exception as e:
        print_error(f"Error sending Telegram message: {e}")
        return False


def send_photo(photo_path, caption=None):
    """Send a photo to Telegram using the HTTP API directly"""
    if not TELEGRAM_TOKEN or not TELEGRAM_CHAT_ID:
        print_error("Telegram not initialized. Call initialize_telegram first.")
        return False

    # Verify the file exists
    if not os.path.exists(photo_path):
        print_error(f"Photo file does not exist: {photo_path}")
        return False

    # Check file size
    try:
        file_size = os.path.getsize(photo_path)
        if file_size == 0:
            print_error(f"Photo file is empty (0 bytes): {photo_path}")
            return False
        print_info(f"Photo file size: {file_size} bytes")
    except Exception as e:
        print_error(f"Error checking file size: {e}")

    # Wait a moment to ensure file is completely written
    time.sleep(0.5)

    # Use with statement to properly handle file closing
    try:
        url = f"https://api.telegram.org/bot{TELEGRAM_TOKEN}/sendPhoto"
        print_info(
            f"Preparing to send photo {os.path.basename(photo_path)} to Telegram..."
        )

        # Open the file with 'with' to ensure proper closing
        with open(photo_path, "rb") as photo_file:
            # Read the file data into memory
            photo_data = photo_file.read()

            if not photo_data:
                print_error(f"Failed to read photo data from {photo_path}")
                return False

            # Prepare the multipart/form-data
            files = {"photo": (os.path.basename(photo_path), photo_data, "image/jpeg")}

            data = {"chat_id": TELEGRAM_CHAT_ID}
            if caption:
                data["caption"] = caption

            # Send the request
            print_info(f"Sending HTTP request to Telegram API...")
            response = requests.post(url, data=data, files=files, timeout=30)

            # Check response
            if response.status_code == 200:
                print_success(
                    f"Telegram photo sent successfully: {os.path.basename(photo_path)}"
                )
                response_json = response.json()
                print_info(f"Telegram API response: {response_json}")
                return True
            else:
                print_error(
                    f"Failed to send photo. Status code: {response.status_code}"
                )
                print_error(f"Response: {response.text}")
                return False
    except Exception as e:
        print_error(f"Error sending Telegram photo: {e}")
        import traceback

        traceback.print_exc()
        return False


def send_fire_alert(alert_id, location=None, sensor_data=None):
    """Send an initial fire alert message with sensor data"""
    global alert_initial_sent

    # Check if we've already sent an initial alert for this alert_id
    if alert_id in alert_initial_sent:
        print_info(f"Initial alert already sent for {alert_id}, skipping")
        return True

    try:
        # Construct the alert message
        message = f"🔥 WILDFIRE ALERT! 🔥\n\nAlert ID: {alert_id}"

        if location:
            message += f"\nLocation: {location}"

        if sensor_data:
            message += "\n\nSensor Data:"
            for key, value in sensor_data.items():
                # Format sensor data nicely
                if key not in ["timestamp", "potential_wildfire"]:
                    if isinstance(value, float):
                        formatted_value = f"{value:.2f}"
                    else:
                        formatted_value = str(value)
                    message += f"\n- {key.capitalize()}: {formatted_value}"

        message += "\n\nStarting continuous monitoring and sending detection images..."

        result = send_message(message)
        if result:
            # Mark this alert as having received its initial message
            alert_initial_sent.add(alert_id)
        return result
    except Exception as e:
        print_error(f"Error sending fire alert: {e}")
        return False
