import cv2
import os
import time

# Function to detect blurriness in an image frame
def detect_blur(frame, threshold=100):
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    laplacian_var = cv2.Laplacian(gray, cv2.CV_64F).var()
    if laplacian_var < threshold:
        return "Blurry", laplacian_var
    else:
        return "Sharp", laplacian_var
    

# Set up webcam
cap = cv2.VideoCapture(0)

# Create 'Temp_Data' folder if it doesn't exist in the same directory as this script
folder_path = os.path.join(os.getcwd(), "Temp_Data")
if not os.path.exists(folder_path):
    os.makedirs(folder_path)
    print(f"Folder '{folder_path}' created.")

# Define the path for saving the video
video_path = os.path.join(folder_path, "CameraTest.avi")

# Remove the existing 'CameraTest.avi' if it exists
if os.path.exists(video_path):
    os.remove(video_path)
    print("Existing 'CameraTest.avi' file removed.")

# Get the frame width and height from the webcam feed
frame_width = int(cap.get(3))  # Width of the frames in the video
frame_height = int(cap.get(4))  # Height of the frames in the video

# Create a VideoWriter object to save the video
fourcc = cv2.VideoWriter_fourcc(*'XVID')  # Codec for the video (XVID for .avi)
out = cv2.VideoWriter(video_path, fourcc, 20.0, (frame_width, frame_height))

# Show the webcam feed window
cv2.namedWindow('Webcam Feed', cv2.WINDOW_NORMAL)

start_time = time.time()

# Counter for frames with variance greater than 100
sharp_count = 0
blur_threshold = 100
variance_values = []  # List to store variance values

while True:
    ret, frame = cap.read()
    if not ret:
        break
    
    # Calculate the current time

    # if elapsed_time < 2:
    #     continue

    # From the 3rd second onwards, calculate the blur value dynamically
    status,laplacian_var = detect_blur(frame, threshold=100)
    variance_values.append(laplacian_var)  # Store the variance value

    cv2.putText(frame, f"Status: {status} - Variance: {laplacian_var:.2f}",
            (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

    # Update the screen with the current variance and status
    if laplacian_var >= blur_threshold:
        sharp_count += 1

    out.write(frame)

    # Show the webcam feed
    cv2.imshow('Webcam Feed', frame)
    elapsed_time = time.time() - start_time
    # After 5 seconds, stop recording and save the video
    if elapsed_time > 5:
        status = "Sharp" if sharp_count > 2 else "Blurry"
        
        print(f"Video Blurriness Test - Status: {status}, Variance:{max(variance_values):.2f}")

        break

    # Add a small delay for the window to refresh properly
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Cleanup
cap.release()
out.release()
cv2.destroyAllWindows()

