import os
import sys
import subprocess
from bs4 import BeautifulSoup

def burn_in_test():
    # Get the desktop path
    desktop_path = os.path.join(os.path.expanduser("~"), "Desktop")

    # Check if desktop exists
    if not os.path.exists(desktop_path):
        print("Desktop path not found!")
        sys.exit(1)

    # Find the first .htm file on the desktop
    htm_files = [f for f in os.listdir(desktop_path) if f.endswith(".htm")]

    if not htm_files:
        return {}

    file_path = os.path.join(desktop_path, htm_files[0])

    # Open and parse the .htm file (Try UTF-16 first, fallback to UTF-8)
    try:
        with open(file_path, "r", encoding="utf-16", errors="replace") as file:
            soup = BeautifulSoup(file, "lxml")
    except UnicodeError:
        with open(file_path, "r", encoding="utf-8", errors="replace") as file:
            soup = BeautifulSoup(file, "lxml")

    # Data labels to extract
    data_labels = [
        "System Model",
        "BIOS Serial Number",
        "BurnInTest Version",
        "Test Start time",
        "Test Stop time",
        "Temperature CPU 0 average (Min/Current/Max)",
        "Temperature CPU 0 core 0 (Min/Current/Max)",
        "Temperature CPU 0 core 1 (Min/Current/Max)",
    ]

    extracted_data = {}

    # Extract general information from tables or paragraphs
    for label in data_labels:
        match = soup.find(string=lambda text: text and label in text)
        if match:
            extracted_text = match.find_next("td").get_text(strip=True) if match.find_next("td") else "Not Found"
            
            # Extract only the max temperature value
            if "Temperature" in label and "/" in extracted_text:
                temp_values = extracted_text.split("/")
                extracted_text = temp_values[-1].strip()  # Take only the max value
            
            extracted_data[label] = extracted_text
        else:
            extracted_data[label] = "Not Found"

    # Extract test results
    test_results = {}
    table_rows = soup.find_all("tr")

    for row in table_rows:
        columns = row.find_all("td")
        if len(columns) >= 5:  # Ensure the row has enough columns
            test_name = columns[0].get_text(strip=True).lower()
            error = columns[4].get_text(strip=True)
            last_error = columns[5].get_text(strip=True)

            if test_name and test_name != "test":
                test_results[test_name] = {"error": error, "last_error": last_error}

    return {"System Info": extracted_data, "Test Results": test_results}


# Run the function and print the results
results = burn_in_test()
print(results)