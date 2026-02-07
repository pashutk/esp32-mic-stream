#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <FastLED.h>
#include <WiFiManager.h>
#include "driver/i2s.h"

static const char* MDNS_HOSTNAME = "esp32-mic";

// LED config (Atom Echo has SK6812 on GPIO 27)
#define LED_PIN 27
#define NUM_LEDS 1
static CRGB leds[NUM_LEDS];

// Button config (Atom Echo has button on GPIO 39 with external pullup)
#define BUTTON_PIN 39

void setLED(uint8_t r, uint8_t g, uint8_t b) {
  leds[0] = CRGB(r, g, b);
  FastLED.show();
}

void configModeCallback(WiFiManager *wm) {
  Serial.println("Entered config mode");
  setLED(50, 0, 50);  // Purple: config portal active
}

bool checkButtonLongPress() {
  static unsigned long pressStart = 0;
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (pressStart == 0) pressStart = millis();
    if (millis() - pressStart > 3000) {
      pressStart = 0;
      return true;
    }
  } else {
    pressStart = 0;
  }
  return false;
}

WebServer server(80);

// Atom Echo PDM mic wiring (verify if needed)
// These are common defaults; adjust if mic levels read as ~0.
static const gpio_num_t MIC_CLK_PIN = GPIO_NUM_33; // PDM clock
static const gpio_num_t MIC_DATA_PIN = GPIO_NUM_34; // PDM data

static const int SAMPLE_RATE_HZ = 16000;
static const int READ_SAMPLES = 320; // 20 ms at 16 kHz

// Audio processing config
static const float HIGHPASS_CUTOFF_HZ = 80.0f;   // Remove low-frequency rumble
static const float LOWPASS_CUTOFF_HZ = 3000.0f;  // Remove high-frequency whine

// Filter state (single-pole IIR for HP, two-pole for LP)
static float hp_prev_in = 0.0f;
static float hp_prev_out = 0.0f;
static float hp_alpha = 0.0f;
static float lp_prev_out1 = 0.0f;
static float lp_prev_out2 = 0.0f;
static float lp_alpha = 0.0f;

void initAudioProcessing() {
  float dt = 1.0f / SAMPLE_RATE_HZ;

  // High-pass: alpha = RC / (RC + dt)
  float hp_rc = 1.0f / (2.0f * PI * HIGHPASS_CUTOFF_HZ);
  hp_alpha = hp_rc / (hp_rc + dt);

  // Low-pass: alpha = dt / (RC + dt)
  float lp_rc = 1.0f / (2.0f * PI * LOWPASS_CUTOFF_HZ);
  lp_alpha = dt / (lp_rc + dt);
}

// Apply band-pass filter (high-pass + two-stage low-pass for steeper rolloff)
void processAudio(int16_t* samples, int count) {
  for (int i = 0; i < count; i++) {
    float in = (float)samples[i];

    // High-pass
    float hp_out = hp_alpha * (hp_prev_out + in - hp_prev_in);
    hp_prev_in = in;
    hp_prev_out = hp_out;

    // Low-pass (two cascaded stages for -12dB/octave rolloff)
    lp_prev_out1 += lp_alpha * (hp_out - lp_prev_out1);
    lp_prev_out2 += lp_alpha * (lp_prev_out1 - lp_prev_out2);

    samples[i] = (int16_t)constrain((int)lp_prev_out2, -32768, 32767);
  }
}

static bool setupMic() {
  i2s_config_t i2s_config = {};
  i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM);
  i2s_config.sample_rate = SAMPLE_RATE_HZ;
  i2s_config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
  i2s_config.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
  i2s_config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  i2s_config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  i2s_config.dma_buf_count = 4;
  i2s_config.dma_buf_len = READ_SAMPLES;
  i2s_config.use_apll = false;
  i2s_config.tx_desc_auto_clear = false;
  i2s_config.fixed_mclk = 0;

  i2s_pin_config_t pin_config = {};
  pin_config.bck_io_num = I2S_PIN_NO_CHANGE;
  pin_config.ws_io_num = MIC_CLK_PIN;
  pin_config.data_out_num = I2S_PIN_NO_CHANGE;
  pin_config.data_in_num = MIC_DATA_PIN;

  if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, nullptr) != ESP_OK) {
    return false;
  }
  if (i2s_set_pin(I2S_NUM_0, &pin_config) != ESP_OK) {
    return false;
  }
  if (i2s_set_clk(I2S_NUM_0, SAMPLE_RATE_HZ, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO) != ESP_OK) {
    return false;
  }
  return true;
}

void handleHello() {
  server.send(200, "text/plain", "hello from esp32-mic\n");
}

// WAV header for 16-bit mono PCM at 16kHz, streaming (unknown length)
void sendWavHeader(WiFiClient& client) {
  const uint32_t sampleRate = SAMPLE_RATE_HZ;
  const uint16_t numChannels = 1;
  const uint16_t bitsPerSample = 16;
  const uint32_t byteRate = sampleRate * numChannels * bitsPerSample / 8;
  const uint16_t blockAlign = numChannels * bitsPerSample / 8;
  const uint32_t dataSize = 0xFFFFFFFF;  // Unknown length for streaming
  const uint32_t fileSize = 36 + dataSize;

  uint8_t header[44];
  // RIFF chunk
  memcpy(header, "RIFF", 4);
  memcpy(header + 4, &fileSize, 4);
  memcpy(header + 8, "WAVE", 4);
  // fmt subchunk
  memcpy(header + 12, "fmt ", 4);
  uint32_t fmtSize = 16;
  memcpy(header + 16, &fmtSize, 4);
  uint16_t audioFormat = 1;  // PCM
  memcpy(header + 20, &audioFormat, 2);
  memcpy(header + 22, &numChannels, 2);
  memcpy(header + 24, &sampleRate, 4);
  memcpy(header + 28, &byteRate, 4);
  memcpy(header + 32, &blockAlign, 2);
  memcpy(header + 34, &bitsPerSample, 2);
  // data subchunk
  memcpy(header + 36, "data", 4);
  memcpy(header + 40, &dataSize, 4);

  client.write(header, 44);
}

void handleStream() {
  WiFiClient client = server.client();

  // Send HTTP headers
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: audio/wav");
  client.println("Connection: close");
  client.println();

  // Send WAV header
  sendWavHeader(client);

  // Stream live audio from mic
  int16_t samples[READ_SAMPLES];
  size_t bytes_read = 0;

  Serial.println("Streaming live audio...");
  setLED(0, 50, 50);  // Cyan: streaming

  while (client.connected()) {
    if (i2s_read(I2S_NUM_0, samples, sizeof(samples), &bytes_read, portMAX_DELAY) == ESP_OK) {
      if (bytes_read > 0) {
        processAudio(samples, bytes_read / sizeof(int16_t));
        client.write((uint8_t*)samples, bytes_read);
      }
    }
  }

  Serial.println("Stream ended");
  setLED(0, 50, 0);  // Green: idle
}

void setup() {
  Serial.begin(115200);
  delay(200);

  // Initialize LED
  FastLED.addLeds<SK6812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(20);
  setLED(0, 0, 50);  // Dim blue: starting up

  // Initialize button
  pinMode(BUTTON_PIN, INPUT);  // External pullup on Atom Echo

  Serial.println("esp32-mic starting");

  // WiFiManager setup
  WiFiManager wm;
  wm.setAPCallback(configModeCallback);
  wm.setConfigPortalTimeout(180);  // 3 min timeout

  setLED(50, 0, 50);  // Purple: config portal (if needed)
  if (!wm.autoConnect("ESP32-Mic-Setup")) {
    Serial.println("Config portal timeout, restarting...");
    ESP.restart();
  }
  Serial.println("WiFi connected!");
  WiFi.mode(WIFI_STA);  // WiFiManager may leave AP active, mDNS needs STA-only
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  setLED(0, 50, 0);  // Green: connected

  // Setup mDNS
  if (MDNS.begin(MDNS_HOSTNAME)) {
    MDNS.addService("http", "tcp", 80);
    Serial.printf("mDNS started: http://%s.local/stream.wav\n", MDNS_HOSTNAME);
  } else {
    Serial.println("mDNS failed to start");
  }

  // Setup HTTP server
  server.on("/", handleHello);
  server.on("/stream.wav", HTTP_GET, handleStream);
  server.begin();
  Serial.println("HTTP server started");

  if (!setupMic()) {
    Serial.println("i2s init failed");
    while (true) {
      setLED(50, 0, 0);  // Red: error
      delay(1000);
    }
  }
  Serial.println("i2s init ok");

  initAudioProcessing();
  Serial.println("audio processing ready");
}

void loop() {
  server.handleClient();

  // Check for long button press to enter config portal
  if (checkButtonLongPress()) {
    Serial.println("Button held - starting config portal");
    setLED(50, 0, 50);  // Purple
    WiFiManager wm;
    wm.setConfigPortalTimeout(180);
    wm.startConfigPortal("ESP32-Mic-Setup");
    WiFi.mode(WIFI_STA);
    MDNS.begin(MDNS_HOSTNAME);
    MDNS.addService("http", "tcp", 80);
    setLED(0, 50, 0);  // Green
  }

  // Update LED based on WiFi status (WiFi auto-reconnects)
  static bool wasConnected = true;
  if (WiFi.status() != WL_CONNECTED) {
    if (wasConnected) {
      Serial.println("WiFi disconnected, reconnecting...");
      setLED(0, 0, 50);  // Blue: reconnecting
      wasConnected = false;
    }
  } else if (!wasConnected) {
    Serial.println("WiFi reconnected");
    MDNS.begin(MDNS_HOSTNAME);
    MDNS.addService("http", "tcp", 80);
    setLED(0, 50, 0);  // Green: connected
    wasConnected = true;
  }
}
