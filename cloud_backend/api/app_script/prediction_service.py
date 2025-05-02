import os
import pickle
import pandas as pd
import numpy as np
from datetime import datetime, timedelta
from tensorflow.keras.models import load_model
import pytz

# Set model directory relative to this file
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
MODEL_DIR = os.path.join(BASE_DIR, "models")
os.makedirs(MODEL_DIR, exist_ok=True)


def make_future_predictions(input_data, steps=24, visualize=False, save_results=False):
    """
    Generate future predictions for all sensors using the trained models.

    Args:
        input_data: DataFrame with at least 60 timesteps of sensor data
                   Must include columns: timestamp, temperature, humidity, smoke, ir_temperature
        steps: Number of future steps to predict (default: 24)
        visualize: Whether to generate visualizations (default: False)
        save_results: Whether to save the results to CSV (default: False)

    Returns:
        DataFrame containing the future predictions for all sensors
    """
    # Ensure input data has enough records
    if len(input_data) < 60:
        raise ValueError(
            f"Input data must contain at least 60 timesteps, but has {len(input_data)}"
        )

    # Make a copy to avoid modifying the original data
    data = input_data.copy()

    # Determine the timezone from the first timestamp
    tz_info = None
    if isinstance(data["timestamp"].iloc[0], str):
        # Parse the timestamp string to get timezone info
        try:
            first_ts = pd.to_datetime(data["timestamp"].iloc[0])
            tz_info = first_ts.tzinfo
            # If timezone wasn't detected but we expect UTC+5:30
            if tz_info is None:
                # Try to extract timezone info from the string directly
                if "+" in data["timestamp"].iloc[0]:
                    # Default to Asia/Kolkata for UTC+5:30
                    tz_info = pytz.timezone("Asia/Kolkata")
                    print("Using Asia/Kolkata timezone (UTC+5:30)")
        except Exception as e:
            print(f"Warning: Could not determine timezone from timestamp: {e}")
            # Default to Asia/Kolkata (UTC+5:30)
            tz_info = pytz.timezone("Asia/Kolkata")

    # Convert timestamp to datetime preserving the timezone
    if isinstance(data["timestamp"].iloc[0], str):
        data["timestamp"] = pd.to_datetime(data["timestamp"], utc=False)
        # If timezone info was determined, apply it to timestamps without timezone
        if tz_info is not None and data["timestamp"].dt.tz is None:
            data["timestamp"] = data["timestamp"].dt.tz_localize(tz_info)

    # Define the numerical columns
    numerical_columns = ["temperature", "humidity", "smoke", "ir_temperature"]

    # Basic preprocessing
    for col in numerical_columns:
        if col in data.columns:
            data[col] = pd.to_numeric(data[col], errors="coerce")

    # Handle any missing values
    data[numerical_columns] = data[numerical_columns].fillna(
        data[numerical_columns].mean()
    )

    # Feature engineering similar to the training process
    df_fe = data.copy()

    # Extract time components
    df_fe["hour"] = df_fe["timestamp"].dt.hour
    df_fe["day"] = df_fe["timestamp"].dt.day
    df_fe["day_of_week"] = df_fe["timestamp"].dt.dayofweek
    df_fe["month"] = df_fe["timestamp"].dt.month
    df_fe["is_day"] = ((df_fe["hour"] >= 6) & (df_fe["hour"] <= 18)).astype(int)

    # Apply cyclical encoding to preserve circular nature of time features
    def cyclical_encoding(df, col, period):
        df[f"{col}_sin"] = np.sin(2 * np.pi * df[col] / period)
        df[f"{col}_cos"] = np.cos(2 * np.pi * df[col] / period)
        return df

    df_fe = cyclical_encoding(df_fe, "hour", 24)
    df_fe = cyclical_encoding(df_fe, "day_of_week", 7)
    df_fe = cyclical_encoding(df_fe, "month", 12)
    df_fe = cyclical_encoding(df_fe, "day", 31)

    # Set timestamp as index temporarily for rolling operations
    df_rolling = df_fe.set_index("timestamp")

    # Generate rolling window features
    window_sizes = [10, 30, 60]  # 10min, 30min, 1hour
    for col in numerical_columns:
        for window in window_sizes:
            # Calculate rolling statistics
            df_rolling[f"{col}_rolling_mean_{window}m"] = (
                df_rolling[col].rolling(f"{window}min").mean()
            )
            df_rolling[f"{col}_rolling_std_{window}m"] = (
                df_rolling[col].rolling(f"{window}min").std()
            )
            df_rolling[f"{col}_rolling_max_{window}m"] = (
                df_rolling[col].rolling(f"{window}min").max()
            )
            df_rolling[f"{col}_rolling_min_{window}m"] = (
                df_rolling[col].rolling(f"{window}min").min()
            )

            # Calculate rate of change over window (first derivative)
            df_rolling[f"{col}_rate_change_{window}m"] = df_rolling[col].diff(window)

            # Calculate acceleration (second derivative) - change in rate of change
            df_rolling[f"{col}_acceleration_{window}m"] = df_rolling[
                f"{col}_rate_change_{window}m"
            ].diff(window)

    # Reset index to get timestamp back as column
    df_fe = df_rolling.reset_index()

    # Create lag features
    df_lagged = df_fe.set_index("timestamp")
    lag_periods = [10, 30, 60]  # 10min, 30min, 1hour
    for col in numerical_columns:
        for lag in lag_periods:
            # Create lagged values
            df_lagged[f"{col}_lag_{lag}m"] = df_lagged[col].shift(freq=f"{lag}min")

            # Calculate difference from lagged value (change over time)
            df_lagged[f"{col}_diff_{lag}m"] = (
                df_lagged[col] - df_lagged[f"{col}_lag_{lag}m"]
            )

            # Calculate percentage change
            df_lagged[f"{col}_pct_change_{lag}m"] = df_lagged[col].pct_change(
                freq=f"{lag}min"
            )

    # Reset index and fill NaN values
    df_fe = df_lagged.reset_index()
    df_fe = df_fe.fillna(df_fe.median())

    # Create domain-specific features
    # Heat-Dryness Index
    df_fe["heat_dryness_index"] = df_fe["temperature"] * (100 - df_fe["humidity"]) / 100

    # Temperature-IR Differential
    df_fe["temp_ir_differential"] = df_fe["ir_temperature"] - df_fe["temperature"]

    # Smoke-Temperature Ratio
    df_fe["smoke_temp_ratio"] = df_fe["smoke"] / df_fe["temperature"]

    # Rapid Temperature Rise Indicator
    temp_std = df_fe["temperature_rolling_std_30m"].replace(
        0, 0.1
    )  # Avoid division by zero
    df_fe["rapid_temp_rise"] = df_fe["temperature_diff_30m"] / temp_std

    # Fire Danger Index
    df_fe["fire_danger_index"] = (
        df_fe["temperature"] * 0.3
        + (100 - df_fe["humidity"]) * 0.3
        + df_fe["smoke"] / 1000 * 0.2
        + df_fe["ir_temperature"] * 0.2
    )

    # Now generate predictions for each target sensor
    predictions = {}
    all_predictions_df = pd.DataFrame()
    sequence_length = 60

    # Generate future timestamps in the same timezone as input data
    last_timestamp = data["timestamp"].iloc[-1]
    # Preserve timezone when generating future timestamps
    future_timestamps = [
        last_timestamp + timedelta(minutes=i + 1) for i in range(steps)
    ]
    all_predictions_df["timestamp"] = future_timestamps

    # Make predictions for each target
    for target in numerical_columns:
        print(f"Generating predictions for {target}...")

        try:
            # Try to load the optimized model first
            model_path = os.path.join(MODEL_DIR, f"optimized_lstm_{target}_model.h5")
            metadata_path = os.path.join(MODEL_DIR, f"fe_lstm_{target}_metadata.pkl")

            if not os.path.exists(model_path):
                # Fall back to feature-engineered model
                model_path = os.path.join(MODEL_DIR, f"fe_lstm_{target}_model.h5")

            # Check if model exists
            if not os.path.exists(model_path) or not os.path.exists(metadata_path):
                print(
                    f"Warning: Model files for {target} not found. Using last value instead."
                )
                last_value = data[target].iloc[-1]
                predictions[target] = np.array([last_value] * steps)
                all_predictions_df[target] = [last_value] * steps
                continue

            # Load model and metadata
            model = load_model(model_path)
            with open(metadata_path, "rb") as f:
                metadata = pickle.load(f)

            # Extract feature columns and scalers
            feature_cols = metadata["feature_cols"]
            scaler_x = metadata["scaler_x"]
            scaler_y = metadata["scaler_y"]

            # Prepare data for prediction
            input_features = df_fe[["timestamp"] + feature_cols + [target]].copy()

            # Handle missing columns (if any)
            missing_cols = set(feature_cols) - set(input_features.columns)
            if missing_cols:
                print(f"Warning: Missing columns for {target}: {missing_cols}")
                for col in missing_cols:
                    input_features[col] = 0.0  # Fill with zeros as fallback

            # Set index and ensure we have the right columns
            input_features = input_features[["timestamp"] + feature_cols + [target]]
            input_features.set_index("timestamp", inplace=True)

            # Scale the features
            scaled_features = input_features.copy()
            try:
                scaled_features[feature_cols] = scaler_x.transform(
                    input_features[feature_cols]
                )
                scaled_features[[target]] = scaler_y.transform(input_features[[target]])
            except Exception as e:
                print(f"Scaling error: {e}")
                print("Trying alternative scaling approach...")
                # Alternative approach if direct transform fails
                for col in feature_cols:
                    scaled_features[col] = (
                        input_features[col] - input_features[col].min()
                    ) / (input_features[col].max() - input_features[col].min() + 1e-10)
                scaled_features[target] = (
                    input_features[target] - input_features[target].min()
                ) / (
                    input_features[target].max() - input_features[target].min() + 1e-10
                )

            # Get the last sequence
            last_sequence = scaled_features.iloc[-sequence_length:].values

            # Generate future predictions
            future_predictions = []
            current_sequence = last_sequence.copy()

            for _ in range(steps):
                # Reshape for prediction
                current_batch = current_sequence.reshape(
                    1, current_sequence.shape[0], current_sequence.shape[1]
                )

                # Predict next value
                next_pred = model.predict(current_batch, verbose=0)[0][0]
                future_predictions.append(next_pred)

                # Update the sequence - create a copy of the last step
                new_step = current_sequence[-1].copy()
                # Find the position of the target column in the features
                target_idx = len(feature_cols)  # Target should be the last column
                new_step[target_idx] = next_pred  # Update the target value

                # Roll the sequence - remove first, add new step at the end
                current_sequence = np.vstack([current_sequence[1:], [new_step]])

            # Convert scaled predictions back to original scale
            future_predictions = np.array(future_predictions).reshape(-1, 1)

            try:
                # Use scaler inverse transform if possible
                original_scale_predictions = scaler_y.inverse_transform(
                    future_predictions
                ).flatten()
            except Exception as e:
                print(f"Inverse scaling error: {e}")
                # Fallback to manual inverse scaling
                min_val = input_features[target].min()
                max_val = input_features[target].max()
                original_scale_predictions = (
                    future_predictions.flatten() * (max_val - min_val) + min_val
                )

            # Store predictions
            predictions[target] = original_scale_predictions
            all_predictions_df[target] = original_scale_predictions

        except Exception as e:
            print(f"Error predicting {target}: {e}")
            # If prediction fails, fill with the last known value
            last_value = data[target].iloc[-1]
            predictions[target] = np.array([last_value] * steps)
            all_predictions_df[target] = [last_value] * steps

    # Calculate Fire Danger Index for predictions
    all_predictions_df["fire_danger_index"] = (
        all_predictions_df["temperature"] * 0.3
        + (100 - all_predictions_df["humidity"]) * 0.3
        + all_predictions_df["smoke"] / 1000 * 0.2
        + all_predictions_df["ir_temperature"] * 0.2
    )

    # Prepare historical data with the same structure as predictions
    historical_data = (
        data[["timestamp", "temperature", "humidity", "smoke", "ir_temperature"]]
        .copy()
        .reset_index(drop=True)
    )

    # Calculate Fire Danger Index for historical data too
    historical_data["fire_danger_index"] = (
        historical_data["temperature"] * 0.3
        + (100 - historical_data["humidity"]) * 0.3
        + historical_data["smoke"] / 1000 * 0.2
        + historical_data["ir_temperature"] * 0.2
    )

    # Add a column to identify data type (historical vs predicted)
    historical_data["data_type"] = "historical"
    all_predictions_df["data_type"] = "predicted"

    # Convert timestamps to string for JSON serialization, preserving timezone info
    if historical_data["timestamp"].dt.tz is not None:
        # Format with timezone info
        historical_data["timestamp"] = historical_data["timestamp"].dt.strftime(
            "%Y-%m-%d %H:%M:%S%z"
        )
        all_predictions_df["timestamp"] = all_predictions_df["timestamp"].dt.strftime(
            "%Y-%m-%d %H:%M:%S%z"
        )
    else:
        # If no timezone info, format without it but add warning
        print("Warning: No timezone information available in timestamps")
        historical_data["timestamp"] = historical_data["timestamp"].dt.strftime(
            "%Y-%m-%d %H:%M:%S"
        )
        all_predictions_df["timestamp"] = all_predictions_df["timestamp"].dt.strftime(
            "%Y-%m-%d %H:%M:%S"
        )

    # Combine historical and predicted data
    combined_df = pd.concat([historical_data, all_predictions_df], ignore_index=True)

    if save_results:
        # Save to CSV
        timestamp_str = datetime.now().strftime("%Y%m%d_%H%M%S")
        csv_filename = f"combined_data_{timestamp_str}.csv"
        combined_df.to_csv(os.path.join(BASE_DIR, csv_filename), index=False)
        print(f"Combined historical and predicted data saved to {csv_filename}")

    return combined_df
