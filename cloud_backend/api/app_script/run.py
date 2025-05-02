import sys
import os

# Add the current directory to the Python path
sys.path.insert(0, os.path.abspath(os.path.dirname(__file__)))

# Check MongoDB connection first
try:
    from db_utils import get_mongodb_client
    from utils.terminal import print_info

    print_info("Checking MongoDB connection...")
    mongo_client = get_mongodb_client()
    if mongo_client:
        print_info("MongoDB connection successful")
except ImportError:
    print("MongoDB utilities not available - continuing without database features")
except Exception as e:
    print(f"Warning: MongoDB connection check failed: {e}")
    print("Continuing with limited functionality")

# Import directly from main (not from app_script.main)
from main import main

if __name__ == "__main__":
    main()
