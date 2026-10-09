#include "audio.h"

#include <Arduino.h>
#include <SPIFFS.h>
#include <driver/i2s.h>

#include "display.h"

// Mikrofon INMP441: D0 = GPIO1, D1 = GPIO2, D2 = GPIO3
#define MIC_I2S_SCK 1
#define MIC_I2S_WS  2
#define MIC_I2S_SD  3
#define MIC_I2S_PORT I2S_NUM_0

static const int sampleRate = 16000;
static const int recordSeconds = 5;
static const int bitsPerSample = 16;
static const int channels = 1;

static const char* wavPath = "/recording.wav";

// Schreibt den WAV-Header an den Anfang der aufgenommenen Datei.
static void writeWavHeader(File fileHandle, uint32_t dataSize) {
  uint32_t fileSize = dataSize + 36;
  uint16_t audioFormat = 1;
  uint32_t byteRate = sampleRate * channels * bitsPerSample / 8;
  uint16_t blockAlign = channels * bitsPerSample / 8;

  fileHandle.seek(0);

  fileHandle.write((const uint8_t*)"RIFF", 4);
  fileHandle.write((uint8_t*)&fileSize, 4);
  fileHandle.write((const uint8_t*)"WAVE", 4);

  fileHandle.write((const uint8_t*)"fmt ", 4);
  uint32_t subchunk1Size = 16;
  fileHandle.write((uint8_t*)&subchunk1Size, 4);
  fileHandle.write((uint8_t*)&audioFormat, 2);
  fileHandle.write((uint8_t*)&channels, 2);
  fileHandle.write((uint8_t*)&sampleRate, 4);
  fileHandle.write((uint8_t*)&byteRate, 4);
  fileHandle.write((uint8_t*)&blockAlign, 2);
  fileHandle.write((uint8_t*)&bitsPerSample, 2);

  fileHandle.write((const uint8_t*)"data", 4);
  fileHandle.write((uint8_t*)&dataSize, 4);
}

// Startet das I2S-Mikrofon.
static bool setupMicI2S() {
  i2s_config_t config = {};
  config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX);
  config.sample_rate = sampleRate;
  config.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
  // Obwohl L/R am Mikrofon auf GND liegt, liefert der ESP32-S3 das Signal
  // mit diesem Treiber ueber den rechten I2S-Kanal.
  config.channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT;
  config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  config.dma_buf_count = 8;
  config.dma_buf_len = 256;
  config.use_apll = false;
  config.tx_desc_auto_clear = false;
  config.fixed_mclk = 0;

  i2s_pin_config_t pins = {};
  pins.bck_io_num = MIC_I2S_SCK;
  pins.ws_io_num = MIC_I2S_WS;
  pins.data_out_num = I2S_PIN_NO_CHANGE;
  pins.data_in_num = MIC_I2S_SD;

  if (i2s_driver_install(MIC_I2S_PORT, &config, 0, nullptr) != ESP_OK) {
    Serial.println("I2S Mikro Start fehlgeschlagen");
    return false;
  }

  if (i2s_set_pin(MIC_I2S_PORT, &pins) != ESP_OK) {
    Serial.println("I2S Pins konnten nicht gesetzt werden");
    i2s_driver_uninstall(MIC_I2S_PORT);
    return false;
  }

  i2s_zero_dma_buffer(MIC_I2S_PORT);

  Serial.println("I2S Mikro gestartet");
  return true;
}

// Nimmt fuer ein paar Sekunden Audio auf und speichert es als WAV in SPIFFS.
bool recordWav() {
  if (!setupMicI2S()) {
    return false;
  }

  if (SPIFFS.exists(wavPath)) {
    SPIFFS.remove(wavPath);
  }

  File fileHandle = SPIFFS.open(wavPath, FILE_WRITE);
  if (!fileHandle) {
    Serial.println("Fehler: WAV-Datei konnte nicht erstellt werden");
    i2s_driver_uninstall(MIC_I2S_PORT);
    return false;
  }

  uint8_t emptyHeader[44] = {0};
  fileHandle.write(emptyHeader, 44);

  const int samplesPerChunk = 256;
  int32_t i2sSamples[samplesPerChunk];
  int16_t pcmSamples[samplesPerChunk];

  uint32_t totalPcmBytes = 0;
  uint32_t totalLoops = (sampleRate * recordSeconds) / samplesPerChunk;

  Serial.println("Aufnahme startet...");
  zeigeNachricht("ICH HOERE", "ZU...");
  delay(150);

  for (uint32_t i = 0; i < totalLoops; i++) {
    size_t bytesRead = 0;
    esp_err_t readResult = i2s_read(
      MIC_I2S_PORT,
      i2sSamples,
      sizeof(i2sSamples),
      &bytesRead,
      portMAX_DELAY
    );

    if (readResult != ESP_OK) {
      Serial.println("I2S Lesen fehlgeschlagen");
      continue;
    }

    int samplesRead = bytesRead / sizeof(int32_t);

    if (samplesRead <= 0) {
      continue;
    }

    for (int j = 0; j < samplesRead; j++) {
      int32_t s = i2sSamples[j] >> 8;
      s = s >> 8;

      if (s > 32767) s = 32767;
      if (s < -32768) s = -32768;

      pcmSamples[j] = (int16_t)s;
    }

    uint32_t pcmBytes = samplesRead * sizeof(int16_t);
    fileHandle.write((uint8_t*)pcmSamples, pcmBytes);
    totalPcmBytes += pcmBytes;
  }

  writeWavHeader(fileHandle, totalPcmBytes);
  fileHandle.close();
  i2s_driver_uninstall(MIC_I2S_PORT);

  File checkFile = SPIFFS.open(wavPath, FILE_READ);
  if (!checkFile) {
    Serial.println("Fehler: WAV-Datei nicht lesbar");
    return false;
  }

  Serial.println("Aufnahme fertig");
  Serial.print("Dateigroesse WAV: ");
  Serial.println(checkFile.size());
  checkFile.close();

  return true;
}
