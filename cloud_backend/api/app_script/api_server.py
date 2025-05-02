import threading
import pandas as pd
from flask import Flask, request, jsonify
from flask_cors import CORS
import traceback
import os
import sys

# Add the current directory to the path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

# Import the prediction function
from prediction_service import make_future_predictions
from utils.terminal import print_info, print_warning, print_error, print_success

# Import MongoDB utilities
from db_utils import (
    fetch_sensor_data,
    resample_to_minute_level,
    get_data_for_prediction,
)

# Create Flask app
app = Flask(__name__)
CORS(app)  # Enable CORS for cross-origin requests


@app.route("/health", methods=["GET"])
def health_check():
    """Simple health check endpoint"""
    return jsonify({"status": "ok", "message": "API server is running"})


@app.route("/predict", methods=["POST"])
def predict():
    """
    Endpoint to make predictions from sensor data

    Expects JSON with at least 60 data points including:
    - timestamp
    - temperature
    - humidity
    - smoke
    - ir_temperature

    Returns 84 data points (60 original + 24 predictions) in JSON format
    """
    try:
        # Get data from request
        data = request.json

        if not data or not isinstance(data, list):
            return (
                jsonify(
                    {
                        "error": "Invalid data format. Expected a JSON array of sensor readings"
                    }
                ),
                400,
            )

        # Check if we have enough data points
        if len(data) < 60:
            return (
                jsonify(
                    {
                        "error": f"Insufficient data points. Need at least 60, got {len(data)}"
                    }
                ),
                400,
            )

        # Convert to DataFrame
        try:
            df = pd.DataFrame(data)
            required_columns = [
                "timestamp",
                "temperature",
                "humidity",
                "smoke",
                "ir_temperature",
            ]

            # Check for required columns
            missing_columns = [col for col in required_columns if col not in df.columns]
            if missing_columns:
                return (
                    jsonify(
                        {
                            "error": f"Missing required columns: {', '.join(missing_columns)}"
                        }
                    ),
                    400,
                )

            print_info(f"Received {len(df)} data points for prediction")

            # Make predictions
            predictions_df = make_future_predictions(
                df, steps=24, visualize=False, save_results=False
            )

            # Convert to dict for JSON serialization
            result = {
                "status": "success",
                "message": "Predictions completed successfully",
                "data": predictions_df.to_dict(orient="records"),
            }

            print_success(
                f"Generated {len(predictions_df)} data points (historical + predictions)"
            )
            return jsonify(result), 200

        except Exception as e:
            print_error(f"Error processing data: {e}")
            traceback.print_exc()
            return jsonify({"error": f"Failed to process data: {str(e)}"}), 500

    except Exception as e:
        print_error(f"Server error: {e}")
        traceback.print_exc()
        return jsonify({"error": f"Server error: {str(e)}"}), 500


@app.route("/predict-from-db", methods=["GET"])
def predict_from_db():
    """
    Endpoint to retrieve data from MongoDB, resample to minute level, and make predictions

    Query parameters:
    - minutes: Number of minutes of historical data to fetch (default: 60)

    Returns 84 data points (60 historical + 24 predictions) in JSON format
    """
    try:
        # Get parameters
        minutes = request.args.get("minutes", default=60, type=int)

        # Validate parameters
        if minutes < 60:
            return (
                jsonify(
                    {"error": f"Minutes parameter must be at least 60, got {minutes}"}
                ),
                400,
            )

        print_info(f"Getting {minutes} minutes of sensor data")

        # Get data for prediction (real or synthetic if necessary)
        data = get_data_for_prediction(minutes=minutes)

        if data is None:
            return (
                jsonify({"error": "Failed to get data for prediction"}),
                500,
            )

        print_info(
            f"Successfully obtained {len(data)} minute-level records for prediction"
        )

        # Make predictions using the data
        predictions_df = make_future_predictions(
            data, steps=24, visualize=False, save_results=False
        )

        # Add data source information
        source_info = "real" if "synthetic" not in data.columns else "synthetic"

        # Convert to dict for JSON serialization
        result = {
            "status": "success",
            "message": f"Predictions completed successfully using {source_info} data",
            "data": predictions_df.to_dict(orient="records"),
        }

        print_success(
            f"Generated {len(predictions_df)} data points (historical + predictions)"
        )
        return jsonify(result), 200

    except Exception as e:
        print_error(f"Error processing data for prediction: {e}")
        traceback.print_exc()
        return jsonify({"error": f"Server error: {str(e)}"}), 500


def start_api_server(host="0.0.0.0", port=5000, debug=False):
    """Start the Flask API server in a separate thread"""
    print_info(f"Starting API server on http://{host}:{port}")
    try:
        # Use threading to run Flask without blocking
        api_thread = threading.Thread(
            target=lambda: app.run(
                host=host, port=port, debug=debug, use_reloader=False
            ),
            daemon=True,
        )
        api_thread.start()
        print_success(f"API server started successfully")
        print_info(f"Prediction endpoint available at http://{host}:{port}/predict")
        print_info(
            f"MongoDB-based prediction endpoint available at http://{host}:{port}/predict-from-db"
        )
        return api_thread
    except Exception as e:
        print_error(f"Failed to start API server: {e}")
        traceback.print_exc()
        return None


if __name__ == "__main__":
    # When run directly, start the server in the main thread
    app.run(host="0.0.0.0", port=5000, debug=True)
