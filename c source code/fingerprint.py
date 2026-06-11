# fingerprint.py
# Checks Windows for biometric/fingerprint device via PnP.
# C++ reads stdout and looks for "working" or "Not Available".

import subprocess
import sys

def main():
    try:
        r = subprocess.run(
            ['powershell', '-NoProfile', '-Command',
             'Get-PnpDevice | Where-Object { '
             '  $_.Class -eq "Biometric" -or '
             '  $_.FriendlyName -like "*fingerprint*" -or '
             '  $_.FriendlyName -like "*finger*" '
             '} | Select-Object -First 1 Status, FriendlyName | ConvertTo-Json'],
            capture_output=True, text=True, timeout=15
        )
        out = r.stdout.strip()

        if not out or out.lower() == 'null':
            print("Not Available")
            return

        if '"OK"' in out or '"ok"' in out.lower():
            print("Fingerprint is working")
        elif '"Error"' in out or '"Unknown"' in out:
            print("Fingerprint Not Working")
        else:
            # Device found but status unclear — treat as working
            print("Fingerprint is working")

    except Exception as e:
        print(f"Not Available: {e}")

if __name__ == "__main__":
    main()
