from datetime import datetime
import colorama
from colorama import Fore, Back, Style
import traceback
import threading

# Initialize colorama (required for Windows)
colorama.init(autoreset=True)


def print_info(message):
    """Print information message in blue"""
    timestamp = datetime.now().strftime("%H:%M:%S")
    thread_id = threading.current_thread().name
    print(f"{Fore.CYAN}[INFO {timestamp} {thread_id}] {message}{Style.RESET_ALL}")


def print_success(message):
    """Print success message in green"""
    timestamp = datetime.now().strftime("%H:%M:%S")
    thread_id = threading.current_thread().name
    print(f"{Fore.GREEN}[SUCCESS {timestamp} {thread_id}] {message}{Style.RESET_ALL}")


def print_warning(message):
    """Print warning message in yellow"""
    timestamp = datetime.now().strftime("%H:%M:%S")
    thread_id = threading.current_thread().name
    print(f"{Fore.YELLOW}[WARNING {timestamp} {thread_id}] {message}{Style.RESET_ALL}")


def print_error(message):
    """Print error message in red"""
    timestamp = datetime.now().strftime("%H:%M:%S")
    thread_id = threading.current_thread().name
    print(f"{Fore.RED}[ERROR {timestamp} {thread_id}] {message}{Style.RESET_ALL}")


def print_alert(message):
    """Print alert message with red background"""
    timestamp = datetime.now().strftime("%H:%M:%S")
    thread_id = threading.current_thread().name
    print(
        f"{Back.RED}{Fore.WHITE}[ALERT {timestamp} {thread_id}] {message}{Style.RESET_ALL}"
    )


def print_header(title):
    """Print section header with blue background"""
    width = 60
    print(f"\n{Back.BLUE}{Fore.WHITE}{title.center(width)}{Style.RESET_ALL}")


def print_separator():
    """Print separator line"""
    print(f"{Fore.BLUE}{'=' * 60}{Style.RESET_ALL}")


def print_exception():
    """Print the current exception with traceback"""
    print(f"{Fore.RED}{'=' * 60}")
    traceback.print_exc()
    print(f"{Fore.RED}{'=' * 60}{Style.RESET_ALL}")


def print_completion_banner(message):
    """Print a completion banner with green background"""
    width = 70
    padding = 1
    border = f"{Back.GREEN}{Fore.BLACK}{'=' * width}{Style.RESET_ALL}"

    print("\n" + border)
    print(
        f"{Back.GREEN}{Fore.BLACK}{' ' * padding}{message.center(width - 2 * padding)}{' ' * padding}{Style.RESET_ALL}"
    )
    print(border + "\n")
