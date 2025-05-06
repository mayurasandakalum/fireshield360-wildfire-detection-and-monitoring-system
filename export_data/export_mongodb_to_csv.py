#!/usr/bin/env python3
"""
Script to export data from MongoDB collection to CSV file
Collection: fireshield360-db.sensor-readings
"""

import os
import sys
import csv
import json
import argparse
from datetime import datetime
import pandas as pd
from pymongo import MongoClient
from pymongo.server_api import ServerApi
from pymongo.errors import ConnectionFailure, OperationFailure

# MongoDB Connection Settings
MONGO_CONNECTION_STRING = (
    "your_mongodb_connection_string"  # Replace with your MongoDB connection string
)
DATABASE_NAME = "your_database_name"  # Replace with your database name
COLLECTION_NAME = "sensor-readings"


def connect_to_mongodb(connection_string):
    """Connect to MongoDB and return database client"""
    try:
        # Create a new client and connect to the server using ServerApi
        client = MongoClient(connection_string, server_api=ServerApi("1"))

        # Verify connection by issuing a server command
        client.admin.command("ping")
        print("Pinged your deployment. You successfully connected to MongoDB!")
        return client
    except Exception as e:
        print(f"Failed to connect to MongoDB: {e}")
        sys.exit(1)


def get_data_from_mongodb(
    client, database_name, collection_name, query=None, limit=None
):
    """Query and retrieve data from MongoDB collection"""
    if query is None:
        query = {}

    db = client[database_name]
    collection = db[collection_name]

    try:
        if limit:
            cursor = collection.find(query).limit(limit)
        else:
            cursor = collection.find(query)

        data = list(cursor)
        print(f"Retrieved {len(data)} documents from collection")
        return data
    except OperationFailure as e:
        print(f"Failed to retrieve data from MongoDB: {e}")
        sys.exit(1)


def export_to_csv(data, output_file):
    """Export MongoDB data to CSV file"""
    if not data:
        print("No data to export")
        return

    try:
        # Convert MongoDB data to DataFrame
        df = pd.DataFrame(data)

        # Drop MongoDB _id column if it exists
        if "_id" in df.columns:
            df["_id"] = df["_id"].astype(str)  # Convert ObjectId to string

        # Create directory if it doesn't exist
        os.makedirs(os.path.dirname(os.path.abspath(output_file)), exist_ok=True)

        # Export to CSV
        df.to_csv(output_file, index=False)
        print(f"Data successfully exported to {output_file}")
    except Exception as e:
        print(f"Error exporting data to CSV: {e}")
        sys.exit(1)


def main():
    parser = argparse.ArgumentParser(description="Export MongoDB data to CSV")
    parser.add_argument(
        "--output",
        "-o",
        default="sensor_readings_export.csv",
        help="Output CSV file path",
    )
    parser.add_argument(
        "--query",
        "-q",
        type=json.loads,
        default={},
        help="MongoDB query in JSON format",
    )
    parser.add_argument(
        "--limit", "-l", type=int, help="Limit the number of documents to export"
    )
    args = parser.parse_args()

    # Add timestamp to filename if not specified
    if args.output == "sensor_readings_export.csv":
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"sensor_readings_export_{timestamp}.csv"
        args.output = filename

    # Connect to MongoDB
    client = connect_to_mongodb(MONGO_CONNECTION_STRING)

    # Get data
    data = get_data_from_mongodb(
        client, DATABASE_NAME, COLLECTION_NAME, query=args.query, limit=args.limit
    )

    # Export to CSV
    export_to_csv(data, args.output)

    # Close MongoDB connection
    client.close()


if __name__ == "__main__":
    main()
