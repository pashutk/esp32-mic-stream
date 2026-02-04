# ESP32 Mic Streamer

![M5Stack Atom Echo](docs/banner.jpg)

ESP32 WiFi microphone streaming over HTTP. Works with M5Stack Atom Echo. Streams 16-bit PCM audio as WAV.

Use as baby monitor, room monitor, or remote mic. No apps, no cloud — just open the stream URL in any audio player (VLC, ffplay, browser, etc.).

```mermaid
flowchart LR
    A[Atom Echo] -->|WiFi| B[Router] -->|HTTP| C[VLC / Browser]
```

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

Open `http://<IP>/stream.wav` in any audio player:

```bash
# VLC
open -a VLC http://<IP>/stream.wav

# ffplay
ffplay http://<IP>/stream.wav

# or just open the URL in a browser
```

## Specs

- 16kHz sample rate
- 16-bit mono PCM
- Band-pass filter (80Hz – 3kHz) to reduce rumble and high-frequency noise
- ~1-2s latency (can be reduced with VLC's `--network-caching` option)
