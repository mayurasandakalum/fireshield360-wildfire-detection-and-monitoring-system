from datetime import datetime
import colorama
from colorama import Fore, Back, Style

# Initialize colorama (required for Windows)
colorama.init(autoreset=True)


def print_info(message):
    """Print information message in blue"""
    timestamp = datetime.now().strftime("%H:%M:%S")
    print(f"{Fore.CYAN}[INFO {timestamp}] {message}{Style.RESET_ALL}")


def print_success(message):
    """Print success message in green"""
    timestamp = datetime.now().strftime("%H:%M:%S")
    print(f"{Fore.GREEN}[SUCCESS {timestamp}] {message}{Style.RESET_ALL}")


def print_warning(message):
    """Print warning message in yellow"""
    timestamp = datetime.now().strftime("%H:%M:%S")
    print(f"{Fore.YELLOW}[WARNING {timestamp}] {message}{Style.RESET_ALL}")


def print_error(message):
    """Print error message in red"""
    timestamp = datetime.now().strftime("%H:%M:%S")
    print(f"{Fore.RED}[ERROR {timestamp}] {message}{Style.RESET_ALL}")


def print_alert(message):
    """Print alert message with red background"""
    timestamp = datetime.now().strftime("%H:%M:%S")
    print(f"{Back.RED}{Fore.WHITE}[ALERT {timestamp}] {message}{Style.RESET_ALL}")


def print_header(title):
    """Print section header with blue background"""
    width = 60
    print(f"\n{Back.BLUE}{Fore.WHITE}{title.center(width)}{Style.RESET_ALL}")


def print_separator():
    """Print separator line"""
    print(f"{Fore.BLUE}{'=' * 60}{Style.RESET_ALL}")
