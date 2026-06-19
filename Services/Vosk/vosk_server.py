import socket
import json
import sys
import os

print("Starting Vosk server...", flush=True)

try:
    from vosk import Model, KaldiRecognizer
    import pyaudio
except ImportError as e:
    print(f"Import error: {e}", flush=True)
    sys.exit(1)

MODEL_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)))
SAMPLE_RATE = 16000

if not os.path.exists(os.path.join(MODEL_PATH, "am")):
    print(f"Model not found at {MODEL_PATH}", flush=True)
    sys.exit(1)

print("Loading Vosk model...", flush=True)
model = Model(MODEL_PATH)
recognizer = KaldiRecognizer(model, SAMPLE_RATE)
print("Model loaded", flush=True)

p = pyaudio.PyAudio()
stream = p.open(format=pyaudio.paInt16, channels=1, rate=SAMPLE_RATE, input=True, frames_per_buffer=4000)

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind(("localhost", 2700))
server.listen(1)

print("Vosk TCP server started on localhost:2700", flush=True)
print("Waiting for connection...", flush=True)

while True:
    client, addr = server.accept()
    print(f"Client connected from {addr}", flush=True)
    stream.start_stream()
    try:
        while True:
            data = stream.read(4000, exception_on_overflow=False)
            if recognizer.AcceptWaveform(data):
                result = json.loads(recognizer.Result())
                text = result.get("text", "")
                if text:
                    print(f"Recognized: {text}", flush=True)
                    try:
                        client.send((text + "\n").encode("utf-8"))
                    except:
                        break
    except (ConnectionResetError, BrokenPipeError):
        print("Client disconnected", flush=True)
    finally:
        stream.stop_stream()
        client.close()