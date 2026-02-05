# ESP32 Mic Streamer

![M5Stack Atom Echo](docs/banner.jpg)

Stream audio from an M5Stack Atom Echo over WiFi. Use as baby monitor, room monitor, or remote mic. No apps, no cloud.

## Getting Started

You need: **M5Stack Atom Echo** + **USB-C cable** + **Chrome or Edge browser**

### 1. Flash the firmware

Open the web installer and click Install:

**https://pashutk.github.io/esp32-mic-stream/**

### 2. Connect to WiFi

- Join the "ESP32-Mic-Setup" WiFi network from your phone or computer
- A setup page opens automatically
- Pick your WiFi network and enter the password

### 3. Start streaming

Open `http://esp32-mic.local/stream.wav` in any audio player:

```bash
# open in browser
open http://esp32-mic.local/stream.wav

# or VLC
open -a VLC http://esp32-mic.local/stream.wav

# or ffplay
ffplay http://esp32-mic.local/stream.wav
```

If `.local` doesn't work on your network, use the IP address instead (shown on the serial monitor or in your router's device list).

That's it. You're done.

---

## LED Status

| Color | Meaning |
|-------|---------|
| Purple | Waiting for WiFi setup |
| Blue | Connecting |
| Green | Ready |
| Cyan | Streaming |
| Red | Error |

## Reconfigure WiFi

Hold the button for 3+ seconds to re-enter WiFi setup mode.

## Development

Build from source with PlatformIO:

```bash
python -m venv .venv && source .venv/bin/activate
pip install platformio
pio run -t upload
```

## Specs

- 16kHz sample rate, 16-bit mono PCM
- Band-pass filter (80Hz – 3kHz)
- Auto-reconnects if WiFi drops
