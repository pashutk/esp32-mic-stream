# Feature Evaluation: ESP32 Mic Streamer

## Project Vibe

This project nails the "small and focused" philosophy:

- **207 lines of code** doing exactly one thing: stream audio over HTTP
- **Zero external dependencies** beyond Arduino framework
- **Works with any standard tool** (VLC, ffplay, browser)
- **Simple setup** - just WiFi credentials
- **No cloud, no app, no accounts**

## Current Features

| Feature | Status |
|---------|--------|
| WiFi connectivity | WPA2 |
| Service discovery | mDNS (`esp32-mic.local`) |
| Audio streaming | HTTP WAV stream |
| Noise filtering | Band-pass (80Hz-3kHz) |
| Documentation | Clear README |

---

## Missing Essentials

These fit the project's vibe and would make it significantly more useful as a headless baby monitor:

### 1. LED Status Indicator (HIGH PRIORITY)

The Atom Echo has an SK6812 NeoPixel LED on **GPIO 27** but it's completely unused. For a headless device, visual feedback is critical:

| State | Current | Should Be |
|-------|---------|-----------|
| Power on | Nothing | Dim pulse or solid color |
| WiFi connecting | Nothing | Blinking blue |
| WiFi connected, idle | Nothing | Solid green |
| Streaming active | Nothing | Breathing cyan |
| Error state | Hangs silently | Solid red |

**Why essential:** Users deploying this as a baby monitor can't tell if it's working without checking serial output. A quick glance should tell you the device status.

### 2. WiFi Reconnection Logic (HIGH PRIORITY)

Current behavior loops forever if WiFi fails initially, and has no recovery if WiFi drops after initial connection.

**Problems:**
- If WiFi drops after initial connection, device becomes unresponsive
- No automatic reconnection requires power cycle
- For a baby monitor that needs reliability, this is a significant gap

**Should have:** Periodic WiFi status check in loop() with automatic reconnection.

### 3. Graceful Error Handling (MEDIUM PRIORITY)

If I2S initialization fails, the device hangs silently with no visual indication.

**Should have:** LED error indication so users know something is wrong.

---

## Nice to Have

These would enhance the project without bloating it:

### 4. Button Support (LOW)

The Atom Echo has a button on **GPIO 39** that's unused. Potential uses:
- Quick press: flash LED to confirm device is alive
- Long press: restart WiFi connection

### 5. Status Endpoint (LOW)

A simple `/status` returning JSON with WiFi RSSI, uptime, streaming status, and free heap memory. Useful for remote monitoring.

### 6. Configurable Gain (LOW)

A compile-time constant to adjust microphone sensitivity for different environments.

---

## Out of Scope

These would bloat the project and go against its minimalist philosophy:

- WiFi AP mode / captive portal configuration
- Web UI for settings
- OTA firmware updates
- Audio compression (Opus, MP3)
- Authentication / HTTPS
- Recording to SD card
- Multiple format support

---

## Summary

| Category | Feature | Priority |
|----------|---------|----------|
| **Essential** | LED status indicator | HIGH |
| **Essential** | WiFi reconnection | HIGH |
| **Essential** | Error indication | MEDIUM |
| Nice to Have | Button support | LOW |
| Nice to Have | Status endpoint | LOW |
| Nice to Have | Gain control | LOW |

The project is well-executed for its scope. Adding **LED feedback** and **WiFi reconnection** would transform it from a "works on my desk" project to a genuinely reliable baby monitor that can be deployed and trusted.
