# ESP32 Mic Streamer

![M5Stack Atom Echo](docs/banner.jpg)

ESP32 WiFi microphone streaming over HTTP. Works with M5Stack Atom Echo. Streams 16-bit PCM audio as WAV.

Use as baby monitor, room monitor, or remote mic. No apps, no cloud — open the stream URL in any audio player (VLC, ffplay, browser, etc.).

```mermaid
flowchart LR
    A[Atom Echo] -- http://esp32-mic.local/stream.wav --> B[VLC / Browser / ffplay]
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

2. Build and flash:
   ```bash
   source .venv/bin/activate
   pio run -t upload
   ```

3. Connect to WiFi:
   - The device creates a WiFi network called "ESP32-Mic-Setup"
   - Connect to it with your phone or computer
   - A captive portal opens automatically (or go to 192.168.4.1)
   - Select your WiFi network and enter the password
   - The device saves credentials and connects automatically

4. (Optional) Check serial monitor to verify it's working:
   ```bash
   pio device monitor
   ```

## Usage

Open `http://esp32-mic.local/stream.wav` in any audio player:

```bash
# VLC
open -a VLC http://esp32-mic.local/stream.wav

# ffplay
ffplay http://esp32-mic.local/stream.wav

# or just open the URL in a browser
```

The device advertises itself via mDNS, so no need to find the IP address. If `.local` doesn't work on your network, check the serial monitor for the IP.

## LED Status

| Color | Meaning |
|-------|---------|
| Purple | Config portal active (waiting for WiFi setup) |
| Blue | Connecting to WiFi |
| Green | Connected, idle |
| Cyan | Streaming audio |
| Red | Error (mic init failed) |

## Button

Hold the button for 3+ seconds to enter WiFi config mode. This lets you reconfigure the WiFi network without reflashing.

## Specs

- 16kHz sample rate
- 16-bit mono PCM
- Band-pass filter (80Hz – 3kHz) to reduce rumble and high-frequency noise
- ~1-2s latency (can be reduced with VLC's `--network-caching` option)
- Auto-reconnects if WiFi drops
