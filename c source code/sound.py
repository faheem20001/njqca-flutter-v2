import sounddevice as sd
import numpy as np
import soundfile as sf

# Record audio output from system using WASAPI loopback
def record_system_audio(duration=5, samplerate=48000):
    try:
        print("Recording system audio output...")
        devices = sd.query_devices()
        loopback_device = None

        for idx, device in enumerate(devices):
            if device['hostapi'] == sd.default.hostapi and device['max_output_channels'] > 0 and 'loopback' in device['name'].lower():
                loopback_device = idx
                break

        if loopback_device is None:
            print("Loopback device not found. Trying default output device.")
            loopback_device = sd.default.device[1]  # fallback to default output

        with sd.InputStream(samplerate=samplerate, channels=2, dtype='float32', device=loopback_device, 
                            blocksize=1024, latency='low', loopback=True) as stream:
            recording = stream.read(int(duration * samplerate))[0]
        print("Recording complete.")
        return recording, samplerate
    except Exception as e:
        print(f"Error during recording: {e}")
        return None, samplerate

# Analyze distortion using clipping ratio and spectral measures
def analyze_audio_data(audio_data, samplerate):
    if audio_data is None:
        print("No audio data to analyze.")
        return

    mono_data = np.mean(audio_data, axis=1)  # Convert to mono

    clipping_ratio = np.mean(np.abs(mono_data) >= 0.99)
    zero_crossings = np.mean(np.abs(np.diff(np.signbit(mono_data))))
    spectrum = np.abs(np.fft.rfft(mono_data)) ** 2
    freqs = np.fft.rfftfreq(len(mono_data), 1 / samplerate)
    centroid = np.sum(freqs * spectrum) / (np.sum(spectrum) + 1e-10)
    flatness = np.exp(np.mean(np.log(spectrum + 1e-10))) / (np.mean(spectrum) + 1e-10)

    print(f"\nAnalysis Results:")
    print(f"Clipping Ratio: {clipping_ratio:.6f}")
    print(f"Zero Crossing Rate: {zero_crossings:.6f}")
    print(f"Spectral Centroid (Hz): {centroid:.2f}")
    print(f"Spectral Flatness: {flatness:.8f}")

    # Decision logic
    if clipping_ratio > 0.01 or flatness > 0.5 or centroid < 500:
        result_text = "SPEAKER IS DISTORTED! (Values shown above)"
    else:
        result_text = "SPEAKER IS OK. (Values shown above)"

    print(f"\nResult: {result_text}")

    # Save results to a text file
    with open("audio_analysis_results.txt", "w") as result_file:
        result_file.write(f"Audio Output Analysis Results:\n")
        result_file.write(f"Clipping Ratio: {clipping_ratio:.6f}\n")
        result_file.write(f"Zero Crossing Rate: {zero_crossings:.6f}\n")
        result_file.write(f"Spectral Centroid (Hz): {centroid:.2f}\n")
        result_file.write(f"Spectral Flatness: {flatness:.8f}\n")
        result_file.write(f"Result: {result_text}\n")

# Save recording for future reference
def save_recording(audio_data, samplerate, filename="system_capture.wav"):
    if audio_data is not None:
        sf.write(filename, audio_data, samplerate)
        print(f"Audio saved to {filename}")

if __name__ == "__main__":
    duration = 5  # Fixed 5 seconds recording for company use
    audio_data, samplerate = record_system_audio(duration)
    save_recording(audio_data, samplerate)
    analyze_audio_data(audio_data, samplerate)
    input("\nPress Enter to close...")
