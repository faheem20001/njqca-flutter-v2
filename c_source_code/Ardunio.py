import serial
import os
from datetime import datetime
import time

# Configuration
SERIAL_PORT = 'COM5'  # Change to your Arduino's port
BAUD_RATE = 115200
LOG_INTERVAL = 1  # Seconds between logs

# Set up CSV file on Desktop
desktop = os.path.join(os.path.join(os.environ['USERPROFILE']), 'Desktop')
csv_file = os.path.join(desktop, f"sensor_data_{datetime.now().strftime('%Y%m%d_%H%M')}.csv")

# CSV header
header = "PC Timestamp,Device Time,Temperature (C),Acceleration (g)\n"

def setup_serial():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2)  # Wait for connection to establish
        ser.flushInput()  # Clear any residual data
        return ser
    except Exception as e:
        print(f"Failed to connect to {SERIAL_PORT}: {e}")
        return None

def main():
    ser = setup_serial()
    if not ser:
        return

    with open(csv_file, 'a') as f:
        f.write(header)
        print(f"Logging data to {csv_file}")
        print("Press Ctrl+C to stop...")
        
        try:
            while True:
                if ser.in_waiting > 0:
                    line = ser.readline().decode('utf-8').strip()
                    if line:  # Only process if we got data
                        pc_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                        f.write(f'"{pc_time}",{line}\n')
                        f.flush()  # Immediate write to disk
                        print(f"Logged: {pc_time} | {line}")
                time.sleep(LOG_INTERVAL)
        except KeyboardInterrupt:
            print("\nLogging stopped by user")
        finally:
            ser.close()

if __name__ == "__main__":
    main()