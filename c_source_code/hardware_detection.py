# import comtypes
# from ctypes import POINTER, cast
# from comtypes import CLSCTX_ALL
# from pycaw.pycaw import AudioUtilities, IAudioEndpointVolume

# # Get all audio devices
# devices = AudioUtilities.GetAllDevices()

# print("Audio Devices Detected:")
# for device in devices:
#     print(f" - {device.FriendlyName} (State: {device.state})")

# # Microphones are usually input devices
# mic_devices = AudioUtilities.GetInputDevices()
# print("\nMicrophones Detected:")
# for mic in mic_devices:
#     print(f" - {mic.FriendlyName}")
import cv2
import warnings
warnings.filterwarnings("ignore")

from pycaw.pycaw import AudioUtilities

# Get all audio devices
devices = AudioUtilities.GetAllDevices()

print("Detected Audio Devices by Category:\n")

speakers = []
microphones = []

for device in devices:
    name = device.FriendlyName
    state = device.state  # 1 = Active, 0 = Disabled, 2 = NotPresent, 4 = Unplugged

    if not name:
        continue

    if "mic" in name.lower():
        microphones.append((name, state))
    elif "speaker" in name.lower() or "audio" in name.lower() or "headphones" in name.lower():
        speakers.append((name, state))

# Display speakers
print("Speakers:")
for name, state in speakers:
    print(f" - {name} (State: {state})")

# Display microphones
print("\nMicrophones:")
for name, state in microphones:
    print(f" - {name} (State: {state})")

index = 0
found = False
while index < 5:  # Check first 5 indexes
    cap = cv2.VideoCapture(index)
    if cap.read()[0]:
        print(f"Camera found at index {index}")
        found = True
        cap.release()
        break
    cap.release()
    index += 1

if not found:
    print("No camera detected")

