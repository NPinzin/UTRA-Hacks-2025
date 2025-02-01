from openai import OpenAI
import wave
import pyaudio
import os

# Parameters
FORMAT = pyaudio.paInt16  # 16-bit audio format
CHANNELS = 1              # Mono audio
RATE = 44100              # Sample rate (Hz)
CHUNK = 1024              # Buffer size (samples per frame)
OUTPUT_FILE = "output.wav"  # Output file path
OPENAI_API_KEY = os.environ.get('OPENAI_API_KEY')

def readAudio():
    # Initialize PyAudio
    p = pyaudio.PyAudio()

    # Open audio stream
    stream = p.open(format=FORMAT,
                    channels=CHANNELS,
                    rate=RATE,
                    input=True,
                    frames_per_buffer=CHUNK)

    frames = []  # List to store audio data

    print("Recording... Press Ctrl+C to stop.")

    try:
        while True:
            data = stream.read(CHUNK)  # Read audio chunk
            frames.append(data)  # Save chunk to list
    except KeyboardInterrupt:
        print("\nStopping...")

    # Cleanup
    stream.stop_stream()
    stream.close()
    p.terminate()

    # Save recorded data to a WAV file
    with wave.open(OUTPUT_FILE, 'wb') as wf:
        wf.setnchannels(CHANNELS)
        wf.setsampwidth(p.get_sample_size(FORMAT))
        wf.setframerate(RATE)
        wf.writeframes(b''.join(frames))  # Write the recorded data
        wf.close()

    print(f"Audio saved as {OUTPUT_FILE}")



def main():
    print(OPENAI_API_KEY)
    readAudio()
    transcribe()


def transcribe(): 
    client = OpenAI()

    audio_file= open("" + OUTPUT_FILE, "rb")
    transcription = client.audio.transcriptions.create(
        model="whisper-1", 
        file=audio_file,
        response_format="json"
    )

    print(transcription.text)

if __name__ == "__main__":
    main()