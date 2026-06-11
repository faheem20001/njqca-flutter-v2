# screen_record.py
# Records screen to Compact_Recording.mkv using ffmpeg gdigrab.
# Runs until killed by: taskkill /im screen_record.exe /f
# Requires working ffmpeg\bin\ffmpeg.exe (64-bit).

import subprocess
import sys
import os

OUTPUT_FILE = "Compact_Recording.mkv"
FFMPEG_PATH = os.path.join("ffmpeg", "bin", "ffmpeg.exe")

def main():
    if not os.path.exists(FFMPEG_PATH):
        print(f"ffmpeg not found at {FFMPEG_PATH} — screen recording skipped.")
        # Create empty file so C++ file-exists check doesn't crash
        open(OUTPUT_FILE, 'wb').close()
        return

    cmd = [
        FFMPEG_PATH, '-y',
        '-f', 'gdigrab',
        '-framerate', '10',
        '-i', 'desktop',
        '-vcodec', 'libx264',
        '-preset', 'ultrafast',
        '-crf', '28',
        OUTPUT_FILE
    ]

    try:
        # Blocks until ffmpeg is killed externally
        subprocess.run(cmd, stdin=subprocess.DEVNULL,
                       stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL)
    except Exception as e:
        print(f"Screen record error: {e}")

if __name__ == "__main__":
    main()
