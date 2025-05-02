import pymongo
import pandas as pd
import pytz
from datetime import datetime, timedelta
from utils.terminal import print_info, print_warning, print_error, print_success

# MongoDB connection parameters - removed trailing slash from URI
MONGO_URI = "mongodb+srv://fireshield360:uHOgfe3X2Lm3usCO@cluster0.aateg5h.mongodb.net/?retryWrites=true&w=majority&appName=Cluster0"
DB_NAME = "fireshield360-db"
COLLECTION_NAME = "sensor-readings"


def get_mongodb_client():
    """Get a MongoDB client connection"""
    try:
        client = pymongo.MongoClient(MONGO_URI)
        # Test the connection
        client.admin.command("ping")
        print_success("Connected to MongoDB")
        return client
    except Exception as e:
        print_error(f"Failed to connect to MongoDB: {e}")
        return None


def get_collection_stats():
    """Get basic statistics about the MongoDB collection to help with debugging"""
    try:
        client = get_mongodb_client()
        if not client:
            return None

        db = client[DB_NAME]
        collection = db[COLLECTION_NAME]

        # Count total documents
        total_docs = collection.count_documents({})

        # Get the newest and oldest timestamps if documents exist
        newest_doc = oldest_doc = None
        newest_time = oldest_time = None

        if total_docs > 0:
            # Get newest document (with highest timestamp)
            newest_doc = collection.find_one(
                {}, sort=[("timestamp", pymongo.DESCENDING)]
            )
            if newest_doc and "timestamp" in newest_doc:
                newest_time = newest_doc["timestamp"]

            # Get oldest document (with lowest timestamp)
            oldest_doc = collection.find_one(
                {}, sort=[("timestamp", pymongo.ASCENDING)]
            )
            if oldest_doc and "timestamp" in oldest_doc:
                oldest_time = oldest_doc["timestamp"]

        client.close()

        stats = {
            "total_documents": total_docs,
            "newest_timestamp": newest_time,
            "oldest_timestamp": oldest_time,
        }

        print_info(f"Collection stats: {stats}")
        return stats

    except Exception as e:
        print_error(f"Error getting collection stats: {e}")
        import traceback

        traceback.print_exc()
        return None


def fetch_sensor_data(minutes=60, end_time=None, fallback_to_available=True):
    """
    Fetch sensor data from MongoDB for the last specified minutes

    Args:
        minutes: Number of minutes of data to fetch (default: 60)
        end_time: End time for the data query (default: current time)
        fallback_to_available: If True, gets available data when no data in specified range

    Returns:
        DataFrame containing the sensor data at second-level granularity
    """
    try:
        client = get_mongodb_client()
        if not client:
            return None

        db = client[DB_NAME]
        collection = db[COLLECTION_NAME]

        # Set end_time to current time if not specified
        if end_time is None:
            end_time = datetime.now()

        # If end_time is a string, convert to datetime
        if isinstance(end_time, str):
            end_time = pd.to_datetime(end_time)

        # Calculate start_time based on minutes parameter
        start_time = end_time - timedelta(minutes=minutes)

        print_info(f"Fetching data from {start_time} to {end_time}")

        # Query MongoDB for data in the time range
        query = {"timestamp": {"$gte": start_time, "$lte": end_time}}

        # Get the data
        cursor = collection.find(query).sort("timestamp", 1)
        data = list(cursor)

        # If no data found and fallback is enabled, get most recent data
        if not data and fallback_to_available:
            print_warning(
                f"No data found between {start_time} and {end_time}. Fetching most recent data."
            )

            # First, check collection stats to understand data availability
            stats = get_collection_stats()

            if stats and stats["total_documents"] > 0:
                # Get most recent data up to the number of minutes requested
                cursor = (
                    collection.find({}).sort("timestamp", -1).limit(minutes * 60)
                )  # Assuming second-level data
                data = list(cursor)
                data.reverse()  # Reverse to get chronological order

                if data:
                    newest_time = data[-1].get("timestamp")
                    oldest_time = data[0].get("timestamp")
                    print_success(
                        f"Found {len(data)} records from {oldest_time} to {newest_time}"
                    )
                else:
                    print_warning("No data found even with fallback approach")

        client.close()

        if not data:
            print_warning(f"No data found in MongoDB")
            return None

        print_success(f"Retrieved {len(data)} records from MongoDB")

        # Convert to DataFrame
        df = pd.DataFrame(data)

        # Ensure timestamp is datetime
        df["timestamp"] = pd.to_datetime(df["timestamp"])

        # Check if we have enough data points after conversion
        if len(df) < 60:
            print_warning(
                f"Only {len(df)} data points found, which is less than the 60 required for prediction"
            )

        return df

    except Exception as e:
        print_error(f"Error fetching data from MongoDB: {e}")
        import traceback

        traceback.print_exc()
        return None


def resample_to_minute_level(df, required_points=60):
    """
    Resample second-level data to minute-level by averaging values within each minute

    Args:
        df: DataFrame with second-level data
        required_points: Minimum number of minute-level points required (default: 60)

    Returns:
        DataFrame with minute-level data
    """
    try:
        if df is None or len(df) == 0:
            print_warning("No data to resample")
            return None

        # Ensure timestamp is set as index for resampling
        if "timestamp" not in df.columns:
            print_error("DataFrame must contain 'timestamp' column for resampling")
            return None

        # Make a copy to avoid modifying the original
        df_copy = df.copy()

        # Check if timezone information is present
        first_ts = df_copy["timestamp"].iloc[0]
        tz_info = first_ts.tzinfo

        # If no timezone, assume UTC+5:30 (Asia/Kolkata)
        if tz_info is None:
            print_info(
                "No timezone information found, assuming Asia/Kolkata (UTC+5:30)"
            )
            tz_info = pytz.timezone("Asia/Kolkata")
            df_copy["timestamp"] = df_copy["timestamp"].dt.tz_localize(tz_info)

        # Set timestamp as index
        df_copy.set_index("timestamp", inplace=True)

        # Define numerical columns for resampling
        numerical_columns = ["temperature", "humidity", "smoke", "ir_temperature"]
        available_columns = [col for col in numerical_columns if col in df_copy.columns]

        if not available_columns:
            print_error(
                f"None of the required numerical columns {numerical_columns} found in data"
            )
            return None

        # Resample to minute frequency using mean
        resampled_df = df_copy[available_columns].resample("1min").mean()

        # Reset index to get timestamp back as column
        resampled_df.reset_index(inplace=True)

        print_success(
            f"Resampled data from {len(df)} second-level records to {len(resampled_df)} minute-level records"
        )

        # Handle missing values
        resampled_df.fillna(method="ffill", inplace=True)  # Forward fill
        resampled_df.fillna(
            method="bfill", inplace=True
        )  # Backward fill for any remaining NAs

        # Check if we have enough minute-level data points
        if len(resampled_df) < required_points:
            print_warning(
                f"Only {len(resampled_df)} minute-level points after resampling, but {required_points} are required"
            )

            # If we have some data but not enough, pad with duplicated data to reach required_points
            if len(resampled_df) > 0:
                print_info(
                    f"Padding data to reach {required_points} points by repeating patterns"
                )

                # Calculate how many times to repeat the data
                multiplier = (required_points // len(resampled_df)) + 1

                # Create extended dataframe
                extended_dfs = []
                last_timestamp = None

                for i in range(multiplier):
                    temp_df = resampled_df.copy()

                    if last_timestamp is not None:
                        # Add minutes to each timestamp based on the previous end time
                        minute_shift = i * len(resampled_df)
                        temp_df["timestamp"] = temp_df["timestamp"].apply(
                            lambda x: x + timedelta(minutes=minute_shift)
                        )

                    extended_dfs.append(temp_df)
                    last_timestamp = temp_df["timestamp"].iloc[-1]

                # Combine all dataframes and take just what we need
                padded_df = pd.concat(extended_dfs, ignore_index=True)
                resampled_df = padded_df.head(required_points)

                print_success(f"Successfully padded data to {len(resampled_df)} points")

        return resampled_df

    except Exception as e:
        print_error(f"Error resampling data: {e}")
        import traceback

        traceback.print_exc()
        return None


# Function to get latest data and generate synthetic data if needed
def get_data_for_prediction(minutes=60):
    """
    Gets data for prediction, falling back to synthetic data generation if needed

    Args:
        minutes: Number of minutes of data to fetch/generate

    Returns:
        DataFrame with minute-level data suitable for prediction
    """
    # First try to get real data from MongoDB
    raw_data = fetch_sensor_data(
        minutes=minutes * 2, fallback_to_available=True
    )  # Get more data than needed

    if raw_data is not None and len(raw_data) > 0:
        # We got some data, resample it to minute level
        resampled_data = resample_to_minute_level(raw_data, required_points=minutes)
        if resampled_data is not None and len(resampled_data) >= minutes:
            # If we have more than the required number of minutes, take only the most recent ones
            if len(resampled_data) > minutes:
                print_info(
                    f"Taking only the most recent {minutes} minute-level records from {len(resampled_data)} available"
                )
                resampled_data = resampled_data.tail(minutes).reset_index(drop=True)
            return resampled_data

    # If we don't have enough data, generate synthetic data
    print_warning(
        f"Insufficient real data available. Generating synthetic data for {minutes} minutes."
    )

    # Generate synthetic data with realistic patterns
    return generate_synthetic_data(minutes)


def generate_synthetic_data(minutes=60):
    """
    Generate synthetic sensor data for testing when real data is unavailable

    Args:
        minutes: Number of minutes of synthetic data to generate

    Returns:
        DataFrame with synthetic minute-level sensor data
    """
    import numpy as np

    # Current time as end point
    end_time = datetime.now()
    if end_time.tzinfo is None:
        end_time = pytz.timezone("Asia/Kolkata").localize(end_time)

    # Generate timestamps, one per minute going back from now
    timestamps = [end_time - timedelta(minutes=i) for i in range(minutes, 0, -1)]

    # Base values
    base_temp = 25.0  # Celsius
    base_humidity = 60.0  # Percentage
    base_smoke = 100.0  # PPM
    base_ir_temp = 27.0  # Celsius

    # Generate data with some randomness and patterns
    np.random.seed(42)  # For reproducibility

    # Time of day effect - higher temperatures during daytime
    hour_of_day = np.array([ts.hour for ts in timestamps])
    daytime_effect = (
        5
        * np.sin(np.pi * (hour_of_day - 6) / 12)
        * (hour_of_day >= 6)
        * (hour_of_day <= 18)
    )

    # Generate sensor values with patterns
    temperature = base_temp + daytime_effect + np.random.normal(0, 1, minutes)

    # Humidity inversely related to temperature
    humidity = base_humidity - 0.5 * daytime_effect + np.random.normal(0, 3, minutes)
    humidity = np.clip(humidity, 30, 90)  # Keep within realistic range

    # Smoke with occasional spikes
    smoke = base_smoke + np.random.normal(0, 10, minutes)
    # Add a few random spikes
    spike_indices = np.random.choice(minutes, size=3, replace=False)
    smoke[spike_indices] += np.random.uniform(50, 200, 3)
    smoke = np.clip(smoke, 50, 500)  # Keep within realistic range

    # IR temperature correlated with regular temperature but higher
    ir_temperature = (
        base_ir_temp + daytime_effect * 1.2 + np.random.normal(0, 1.5, minutes)
    )

    # Create DataFrame
    df = pd.DataFrame(
        {
            "timestamp": timestamps,
            "temperature": temperature,
            "humidity": humidity,
            "smoke": smoke,
            "ir_temperature": ir_temperature,
        }
    )

    print_info(f"Generated {minutes} minutes of synthetic sensor data")
    return df
