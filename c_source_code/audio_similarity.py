import sounddevice as sd
import soundfile as sf
import numpy as np
import os
import pygame
import threading
import tkinter as tk
from tkinter import messagebox
import time
import wave
import sys
from scipy.io import wavfile
import pandas as pd
# Check if ffmpeg is installed and install it if necessary

# Create necessary directories
def ensure_directory_exists(directory):
    if not os.path.exists(directory):
        os.makedirs(directory)
        print(f"Directory {directory} created.")
# Generate a test tone if the audio file doesn't exist (without using pydub)
def generate_test_tone(file_path, duration=3, freq=440):
    ensure_directory_exists(os.path.dirname(file_path))
    try:
        # Parameters for the sine wave
        sample_rate = 44100  # Standard audio sample rate
        t = np.linspace(0, duration, int(sample_rate * duration), False)
        tone = 0.5 * np.sin(2 * np.pi * freq * t)  # 0.5 is the amplitude  
        # Convert to 16-bit PCM
        tone = (tone * 32767).astype(np.int16)    
        # Write to WAV file
        with wave.open(file_path, 'wb') as wf:
            wf.setnchannels(1)  # Mono
            wf.setsampwidth(2)  # 16-bit
            wf.setframerate(sample_rate)
            wf.writeframes(tone.tobytes())
        
        print(f"Test tone generated at {file_path}")
        return True
    except Exception as e:
        print(f"Error generating test tone: {e}")
        return False

def show_yes_no_popup(message, audio_file, speaker="left"):
    stop_playback = threading.Event()
    if speaker == "left":
        playback_thread = threading.Thread(target=play_audio_in_left_speaker_loop, args=(audio_file, stop_playback))
    elif speaker == "right":
        playback_thread = threading.Thread(target=play_audio_in_right_speaker_loop, args=(audio_file, stop_playback))    
    playback_thread.start()
    root = tk.Tk()  
    root.withdraw()  
    root.attributes("-topmost", 1)  # Make the popup appear on top of all other windows
    result = messagebox.askyesno("Speaker Check", message)
    root.destroy()  # Destroy the root window after showing the popup
    stop_playback.set()
    playback_thread.join()
    return result

def play_audio_in_left_speaker(audio_path, stop_playback_event):
    pygame.mixer.init()
    sound = pygame.mixer.Sound(audio_path)
    channel = pygame.mixer.find_channel()
    channel.set_volume(1.0, 0.0)  # Play only in the left speaker
    channel.play(sound)  
    while channel.get_busy():
        if stop_playback_event.is_set():  # Check if the event has been triggered
            channel.stop()  # Stop the playback immediately
            break
        pygame.time.Clock().tick(10)

def play_audio_in_right_speaker(audio_path, stop_playback_event):
    pygame.mixer.init()
    sound = pygame.mixer.Sound(audio_path)
    channel = pygame.mixer.find_channel()
    channel.set_volume(0.0, 1.0)  # Play only in the right speaker
    channel.play(sound)
    while channel.get_busy():
        if stop_playback_event.is_set():  # Check if the event has been triggered
            channel.stop()  # Stop the playback immediately
            break
        pygame.time.Clock().tick(10)

def play_audio_in_left_speaker_loop(audio_path, stop_playback_event):
    pygame.mixer.init()
    sound = pygame.mixer.Sound(audio_path)
    channel = pygame.mixer.find_channel()
    channel.set_volume(1.0, 0.0)  # Play only in the left speaker
    channel.play(sound, loops=-1)  # Loop the sound indefinitely after the popup is shown 
    while not stop_playback_event.is_set():
        pygame.time.Clock().tick(10)  # Check every 10 ms if the stop event is set
    channel.stop()  # Stop the playback when the stop event is set

def play_audio_in_right_speaker_loop(audio_path, stop_playback_event):
    pygame.mixer.init()
    sound = pygame.mixer.Sound(audio_path)
    channel = pygame.mixer.find_channel()
    channel.set_volume(0.0, 1.0)  # Play only in the right speaker
    channel.play(sound, loops=-1)  # Loop the sound indefinitely after the popup is shown
    while not stop_playback_event.is_set():
        pygame.time.Clock().tick(10)  # Check every 10 ms if the stop event is set
    channel.stop()  # Stop the playback when the stop event is set
def calculate_dBFS(audio_file):
    try:
        # Read the wav file
        with wave.open(audio_file, 'rb') as wf:
            # Get audio parameters
            n_channels = wf.getnchannels()
            sample_width = wf.getsampwidth()
            n_frames = wf.getnframes()     
            # Read frames
            frames = wf.readframes(n_frames)      
            # Convert binary data to numpy array
            if sample_width == 1:  # 8-bit samples
                dtype = np.uint8
                # Convert from unsigned to signed
                data = np.frombuffer(frames, dtype=dtype)
                data = (data.astype(np.int16) - 128) * 256
            elif sample_width == 2:  # 16-bit samples
                dtype = np.int16
                data = np.frombuffer(frames, dtype=dtype)
            else:
                raise ValueError(f"Unsupported sample width: {sample_width}")  
            # Reshape for multiple channels
            if n_channels > 1:
                data = data.reshape(-1, n_channels)
                # Convert to mono by averaging channels
                data = data.mean(axis=1)     
            # Calculate RMS (root mean square)
            rms = np.sqrt(np.mean(np.square(data, dtype=np.float64)))     
            # Avoid log of zero
            if rms < 1e-10:
                return -float('inf')          
            # Convert to dBFS
            max_possible_amplitude = float(2 ** (sample_width * 8 - 1))
            dbfs = 20 * np.log10(rms / max_possible_amplitude)         
            return dbfs
    except Exception as e:
        print(f"Error calculating dBFS: {e}")
        return -float('inf')

def play_and_record(audio_file, output_file, speaker="left", duration=3):
    if not os.path.exists(audio_file):
        print(f"Audio file {audio_file} not found")
        if not generate_test_tone(audio_file):
            return None   
    try:
        data, samplerate = sf.read(audio_file)    
        # Calculate duration based on the audio file
        audio_duration = len(data) / samplerate    
        # Ensure the output directory exists
        ensure_directory_exists(os.path.dirname(output_file))
        # Get available devices
        devices = sd.query_devices()
        input_devices = [device for device in devices if device['max_input_channels'] > 0]       
        if not input_devices:
            print("No input devices found. Please check your microphone settings.")
            return None        
        try:
            selected_device = input_devices[0]['name']
            print(f"Using input device: {selected_device}")
        except IndexError:
            print("Could not select an input device. Using default.")
            selected_device = None  
        # Create a stop event for the playback thread
        stop_event = threading.Event()     
        with sf.SoundFile(output_file, mode='w', samplerate=samplerate, channels=1) as out_file:
            def callback(indata, frames, time, status):
                if status:
                    print(f"Status: {status}")
                out_file.write(indata)      
            # Start recording
            with sd.InputStream(callback=callback, channels=1, samplerate=samplerate, device=selected_device):
                print(f"Starting playback in {speaker} speaker and recording...")          
                # Start playback in a separate thread
                if speaker == "left":
                    playback_thread = threading.Thread(target=play_audio_in_left_speaker, args=(audio_file, stop_event))
                elif speaker == "right":
                    playback_thread = threading.Thread(target=play_audio_in_right_speaker, args=(audio_file, stop_event))
                
                playback_thread.start()             
                # Record for the duration of the audio file plus a small buffer
                time.sleep(audio_duration + 0.5)               
                # Stop the playback
                stop_event.set()
                playback_thread.join()            
        return output_file
    
    except Exception as e:
        print(f"An error occurred during recording: {e}")
        return None


def main(audio_file, similarity_threshold=65):
    # Ensure the audio file exists
    if not os.path.exists(audio_file):
        print(f"Audio file {audio_file} not found. Generating a test tone...")
        if not generate_test_tone(audio_file):
            print("Failed to generate test tone. Exiting.")
            return    
    # Create necessary directories
    # Test left speaker
    left_output_file = './Temp_Data/recorded_audio_left.wav'
    recorded_audio_left = play_and_record(audio_file, left_output_file, speaker="left")
    
    if recorded_audio_left:
        left_dBFS = calculate_dBFS(recorded_audio_left)
        print(f"Left speaker dBFS: {left_dBFS:.2f} dBFS")
        
    right_output_file = './Temp_Data/recorded_audio_right.wav'
    recorded_audio_right = play_and_record(audio_file, right_output_file, speaker="right")  
   


def analyze_audio_distortion(filepath):
    sample_rate, data = wavfile.read(filepath)

    # Normalize and convert to float
    if data.dtype != np.float32:
        data = data.astype(np.float32) / np.max(np.abs(data))

    # Convert to mono if stereo
    if len(data.shape) == 2:
        data = np.mean(data, axis=1)

    # Calculate Peak-to-RMS Ratio
    peak = np.max(np.abs(data))
    rms = np.sqrt(np.mean(data ** 2))
    peak_rms_ratio = 20 * np.log10(peak / rms) if rms > 0 else 0

    # Clipping Ratio
    clipping_ratio = np.sum(np.abs(data) >= 0.999) / len(data)

    # Spectral Flatness
    spectrum = np.abs(np.fft.rfft(data))
    spectral_flatness = (
        np.exp(np.mean(np.log(spectrum + 1e-10))) / 
        (np.mean(spectrum) + 1e-10)
    )

    # Frequency array and fundamental
    frequencies = np.fft.rfftfreq(len(data), 1 / sample_rate)
    peak_index = np.argmax(spectrum)
    fundamental_freq = frequencies[peak_index]

    def get_harmonic_db(harmonic):
        target_freq = harmonic * fundamental_freq
        idx = np.argmin(np.abs(frequencies - target_freq))
        harmonic_mag = spectrum[idx]
        base_mag = spectrum[peak_index]
        if base_mag > 0 and harmonic_mag > 0:
            return 20 * np.log10(harmonic_mag / base_mag)
        return -100.0

    harmonic2_db = get_harmonic_db(2)
    harmonic3_db = get_harmonic_db(3)

    # Verdict based on stricter QC thresholds
    failed = []
    # if clipping_ratio > 0.005:
    #     failed.append("Clipping")
    # if peak_rms_ratio < 3.5:
    #     failed.append("RMS")
    # if spectral_flatness > 0.25:
    #     failed.append("Flatness")
    if harmonic2_db > -22:
        failed.append("2nd Harmonic")
    if harmonic3_db > -25:
        failed.append("3rd Harmonic")

    verdict = "PASS" if not failed else "FAIL: " + ", ".join(failed)

    return {
        "File": os.path.basename(filepath),
        # "Clipping Ratio": round(clipping_ratio, 3),
        # "Peak-to-RMS Ratio (dB)": round(peak_rms_ratio, 3),
        # "Spectral Flatness": round(spectral_flatness, 3),
        "2nd Harmonic dB": round(harmonic2_db, 3),
        "3rd Harmonic dB": round(harmonic3_db, 3),
        "Distortion Verdict": verdict
    }

def calculate_rms(filepath):
    sample_rate, data = wavfile.read(filepath)

    if data.ndim > 1:
        data = data[:, 0]

    if np.issubdtype(data.dtype, np.integer):
        data = data / np.iinfo(data.dtype).max

    rms = np.sqrt(np.mean(data**2))
    return rms
if __name__ == "__main__":
    # Audio file path
    audio_path = './Temp_Data/test_tone.wav'
    main(audio_path)
    folder = './Temp_Data'
    wav_files = ['./Temp_Data/recorded_audio_left.wav', './Temp_Data/recorded_audio_right.wav']
    if not wav_files:
        print("No .wav files found in the folder.")
        sys.exit(1)

    all_results = []
    for filepath in wav_files:
        result = analyze_audio_distortion(filepath)
        rms_value = calculate_rms(filepath)
        print(f"{os.path.basename(filepath)}  RMS Value: {rms_value:.3f}")
        all_results.append(result)

    df = pd.DataFrame(all_results)
    # Format floats to 3 decimals
    pd.options.display.float_format = '{:.3f}'.format
    print(df.iloc[:, :4].to_string(index=True))
    print(df.iloc[:, 4:].to_string(index=True))
    df.to_csv("distortion_report.csv", index=False)
