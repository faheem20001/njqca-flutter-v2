# restart.py
# Reads restart times already in Temp_Data/restart_times.txt,
# calculates average, and appends:  Average Restart Interval: H:MM:SS
# C++ calls this as: average_restart_command = "restart.exe"

import re
import os

FILE_PATH = "Temp_Data/restart_times.txt"

def parse_seconds(h, m, s):
    return int(h) * 3600 + int(m) * 60 + int(s)

def format_hms(total):
    h = total // 3600
    m = (total % 3600) // 60
    s = total % 60
    return f"{h}:{m:02d}:{s:02d}"

def main():
    if not os.path.exists(FILE_PATH):
        print(f"File not found: {FILE_PATH}")
        return

    with open(FILE_PATH, 'r') as f:
        content = f.read()

    # Already calculated — skip
    if 'Average Restart Interval:' in content:
        print("Average Restart Interval already present.")
        return

    matches = re.findall(r'restart \d+:(\d+):(\d+):(\d+)', content)
    if not matches:
        print("No restart times found in file.")
        return

    total_seconds = sum(parse_seconds(h, m, s) for h, m, s in matches)
    avg = total_seconds // len(matches)
    avg_str = format_hms(avg)

    with open(FILE_PATH, 'a') as f:
        f.write(f"\nAverage Restart Interval: {avg_str}\n")

    print(f"Average Restart Interval: {avg_str}")

if __name__ == "__main__":
    main()
