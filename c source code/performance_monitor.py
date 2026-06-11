# performance_monitor.py
# Collects hardware sensor data and writes to Desktop\HardwareMonitor_Report.csv
# Column names must match exactly what C++ performanceCheck.h expects.
# Runs until killed by: taskkill /im performance_monitor.exe /f

import csv
import os
import time
import subprocess
import psutil

COLUMNS = [
    "Disk Read KB/s",
    "Disk Write KB/s",
    " | Load | Memory",
    " | Temperature | CPU Package",
    " | Load | CPU Total",
    "Charge Rate"
]

def get_cpu_temp():
    try:
        r = subprocess.run(
            ['powershell', '-NoProfile', '-Command',
             '(Get-WmiObject MSAcpi_ThermalZoneTemperature -Namespace "root/wmi" '
             '| Select-Object -First 1).CurrentTemperature'],
            capture_output=True, text=True, timeout=5
        )
        raw = r.stdout.strip()
        if raw and raw.lstrip('-').isdigit():
            return round(float(raw) / 10.0 - 273.15, 2)
    except Exception:
        pass
    return 0.0

def get_charge_rate():
    try:
        b = psutil.sensors_battery()
        if b:
            return 1.0 if b.power_plugged else 0.0
    except Exception:
        pass
    return 0.0

def main():
    user_profile = os.environ.get('USERPROFILE', os.path.expanduser('~'))
    output_path = os.path.join(user_profile, 'Desktop', 'HardwareMonitor_Report.csv')

    prev_disk = psutil.disk_io_counters()
    # warm up cpu_percent
    psutil.cpu_percent(interval=None)

    with open(output_path, 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=COLUMNS)
        writer.writeheader()
        f.flush()

        while True:
            try:
                time.sleep(1)
                curr_disk = psutil.disk_io_counters()
                read_kb  = (curr_disk.read_bytes  - prev_disk.read_bytes)  / 1024.0
                write_kb = (curr_disk.write_bytes - prev_disk.write_bytes) / 1024.0
                prev_disk = curr_disk

                mem_load  = psutil.virtual_memory().percent
                cpu_load  = psutil.cpu_percent(interval=None)
                cpu_temp  = get_cpu_temp()
                charge    = get_charge_rate()

                writer.writerow({
                    "Disk Read KB/s":             round(read_kb,  2),
                    "Disk Write KB/s":            round(write_kb, 2),
                    " | Load | Memory":           round(mem_load, 2),
                    " | Temperature | CPU Package": round(cpu_temp, 2),
                    " | Load | CPU Total":        round(cpu_load, 2),
                    "Charge Rate":                round(charge,   2),
                })
                f.flush()
            except Exception:
                pass

if __name__ == "__main__":
    main()
