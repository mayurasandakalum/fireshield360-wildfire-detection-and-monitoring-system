import sys
import os

# Add the current directory to the Python path
sys.path.insert(0, os.path.abspath(os.path.dirname(__file__)))

# Import directly from main (not from app_script.main)
from main import main

if __name__ == "__main__":
    main()
