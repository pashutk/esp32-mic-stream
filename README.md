# ESP32 Mic Streamer

Stream live audio from M5Stack Atom Echo to VLC over WiFi.

Simple DIY baby monitor — place the device in a room and listen from anywhere on your local network. No apps, no cloud, just VLC.

Built with Claude Code.

## Hardware

- M5Stack Atom Echo (ESP32-PICO-D4 with PDM mic)

## Setup

1. Install PlatformIO:
   ```bash
   python -m venv .venv
   source .venv/bin/activate
   pip install platformio
   ```

2. Copy and fill in your WiFi credentials:
   ```bash
   cp src/credentials.h.example src/credentials.h
   # edit src/credentials.h with your SSID and password
   ```

3. Build and flash:
   ```bash
   source .venv/bin/activate
   pio run -t upload
   ```

4. Get the IP address from serial monitor:
   ```bash
   pio device monitor
   ```

## Usage

Open the stream in VLC:
```bash
open -a VLC http://<IP>/stream.wav
```

Or: VLC > File > Open Network > `http://<IP>/stream.wav`

## Specs

- 16kHz sample rate
- 16-bit mono PCM
- Band-pass filter (80Hz – 3kHz) to reduce rumble and high-frequency noise
- ~1-2s latency (can be reduced with VLC's `--network-caching` option)
