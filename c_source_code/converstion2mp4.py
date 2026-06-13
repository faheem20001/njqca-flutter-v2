# For MoviePy v2.0+ (The current version)
# from moviepy import VideoFileClip

# def convert_mkv_to_mp4(input_path, output_path):
#     try:
#         # Load and convert
#         with VideoFileClip(input_path) as video:
#             video.write_videofile(output_path, codec="libx264")
#         print("Success!")
#     except Exception as e:
#         print(f"Error: {e}")

# convert_mkv_to_mp4("Compact_Recording.mkv", "output.mp4")

import subprocess
import os

def convert_video(input_name, output_name):
    # The command we are sending to the system
    # -i: input file
    # -vcodec libx264: converts video to standard MP4 format
    # -acodec aac: converts audio to standard MP4 format
    # -y: overwrite if output exists
    command = [
        'ffmpeg', 
        '-i', input_name, 
        '-vcodec', 'libx264', 
        '-acodec', 'aac', 
        '-y', 
        output_name
    ]
    
    print(f"Processing {input_name}...")
    
    try:
        # Run the command and capture errors
        result = subprocess.run(command, capture_output=True, text=True)
        
        if result.returncode == 0:
            print(f"Success! Created: {output_name}")
        else:
            print("FFmpeg Error:")
            print(result.stderr) # This tells us exactly why it failed
            
    except FileNotFoundError:
        print("Error: FFmpeg is not installed or not in your Windows PATH.")

# Run the function
convert_video("Compact_Recording.mkv", "Compact_Recording.mp4")