#include "assistant.h"

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#include "secrets.h"
#include "display.h"
#include "audio.h"

static const char* wavPath = "/recording.wav";

// Schiebt die WAV-Datei zu Gemini und holt die file_uri zurueck.
static bool uploadFileToGemini(String& fileUri, String& mimeTypeOut) {
  File audioFile = SPIFFS.open(wavPath, FILE_READ);
  if (!audioFile) {
    Serial.println("WAV-Datei nicht gefunden");
    return false;
  }

  size_t fileSize = audioFile.size();
  String mimeType = "audio/wav";
  mimeTypeOut = mimeType;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  const char* headerKeys[] = {"x-goog-upload-url"};
  https.collectHeaders(headerKeys, 1);

  String startUrl = "https://generativelanguage.googleapis.com/upload/v1beta/files";
  if (!https.begin(client, startUrl)) {
    Serial.println("Gemini Upload-Start begin fehlgeschlagen");
    audioFile.close();
    return false;
  }

  https.addHeader("x-goog-api-key", geminiApiKey);
  https.addHeader("X-Goog-Upload-Protocol", "resumable");
  https.addHeader("X-Goog-Upload-Command", "start");
  https.addHeader("X-Goog-Upload-Header-Content-Length", String((unsigned int)fileSize));
  https.addHeader("X-Goog-Upload-Header-Content-Type", mimeType);
  https.addHeader("Content-Type", "application/json");

  String metaBody = "{\"file\":{\"display_name\":\"recording.wav\"}}";
  int startCode = https.POST(metaBody);

  Serial.print("Gemini Upload-Start Status: ");
  Serial.println(startCode);

  if (startCode <= 0) {
    Serial.println("Gemini Upload-Start fehlgeschlagen");
    https.end();
    audioFile.close();
    return false;
  }

  String uploadUrl = https.header("x-goog-upload-url");
  https.end();

  if (uploadUrl.length() == 0) {
    Serial.println("Keine x-goog-upload-url erhalten");
    audioFile.close();
    return false;
  }

  HTTPClient uploadHttp;
  if (!uploadHttp.begin(client, uploadUrl)) {
    Serial.println("Gemini Upload begin fehlgeschlagen");
    audioFile.close();
    return false;
  }

  uploadHttp.addHeader("Content-Length", String((unsigned int)fileSize));
  uploadHttp.addHeader("X-Goog-Upload-Offset", "0");
  uploadHttp.addHeader("X-Goog-Upload-Command", "upload, finalize");

  int uploadCode = uploadHttp.sendRequest("POST", &audioFile, fileSize);
  Serial.print("Gemini Upload Status: ");
  Serial.println(uploadCode);

  audioFile.close();

  if (uploadCode <= 0) {
    Serial.println("Gemini Datei-Upload fehlgeschlagen");
    uploadHttp.end();
    return false;
  }

  String responseBody = uploadHttp.getString();
  uploadHttp.end();

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, responseBody);
  if (error) {
    Serial.print("Gemini Upload JSON Fehler: ");
    Serial.println(error.c_str());
    return false;
  }

  const char* uri = doc["file"]["uri"];
  if (!uri) {
    Serial.println("Keine file.uri gefunden");
    return false;
  }

  fileUri = String(uri);
  return true;
}

// Baut die eigentliche Anfrage an Gemini und liest die Antwort aus.
static String frageGeminiMitDatei(const String& fileUri, const String& mimeType) {
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(25000);

  if (!client.connect("generativelanguage.googleapis.com", 443)) {
    Serial.println("Gemini HTTPS Verbindung fehlgeschlagen");
    return "";
  }

  String prompt =
    "Deine Antwort wird auf einem kleinen OLED-Display mit 128 mal 64 Pixeln angezeigt. "
    "Höre genau auf die hochgeladene Aufnahme und beantworte ausschließlich die darin gestellte Frage. "
    "Erfinde keine Begrüßung und keine Frage. "
    "Wenn die Sprache nicht verständlich ist, antworte nur: Ich habe dich nicht verstanden. "
    "Antworte auf Deutsch mit höchstens 70 Zeichen und ohne Aufzählungen oder Formatierungen.";

  JsonDocument doc;
  JsonArray contents = doc["contents"].to<JsonArray>();
  JsonObject content0 = contents.add<JsonObject>();
  JsonArray parts = content0["parts"].to<JsonArray>();

  JsonObject textPart = parts.add<JsonObject>();
  textPart["text"] = prompt;

  JsonObject filePart = parts.add<JsonObject>();
  JsonObject fileData = filePart["file_data"].to<JsonObject>();
  fileData["mime_type"] = mimeType;
  fileData["file_uri"] = fileUri;

  JsonObject genCfg = doc["generationConfig"].to<JsonObject>();
  genCfg["temperature"] = 0.1;
  genCfg["maxOutputTokens"] = 60;

  JsonObject thinkingCfg = genCfg["thinkingConfig"].to<JsonObject>();
  thinkingCfg["thinkingLevel"] = "minimal";

  String requestBody;
  serializeJson(doc, requestBody);

  String path = "/v1beta/models/" + String(geminiModel) +
                ":generateContent?key=" + String(geminiApiKey);

  client.printf("POST %s HTTP/1.1\r\n", path.c_str());
  client.printf("Host: generativelanguage.googleapis.com\r\n");
  client.printf("Content-Type: application/json\r\n");
  client.printf("Accept: application/json\r\n");
  client.printf("Connection: close\r\n");
  client.printf("Content-Length: %u\r\n\r\n", (unsigned int)requestBody.length());
  client.print(requestBody);

  unsigned long startWait = millis();
  while (!client.available() && client.connected() && millis() - startWait < 25000) {
    delay(10);
  }

  if (!client.available()) {
    Serial.println("Gemini: Keine Antwort erhalten");
    return "";
  }

  String statusLine = client.readStringUntil('\n');
  statusLine.trim();
  Serial.print("Gemini Status: ");
  Serial.println(statusLine);

  bool chunked = false;
  int contentLength = -1;

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    line.trim();

    if (line.startsWith("Transfer-Encoding:") && line.indexOf("chunked") >= 0) {
      chunked = true;
    }

    if (line.startsWith("Content-Length:")) {
      contentLength = line.substring(String("Content-Length:").length()).toInt();
    }

    if (line.length() == 0) {
      break;
    }
  }

  String responseBody = "";

  if (chunked) {
    while (true) {
      while (!client.available() && client.connected()) {
        delay(1);
      }

      String chunkSizeLine = client.readStringUntil('\n');
      chunkSizeLine.trim();

      if (chunkSizeLine.length() == 0) {
        continue;
      }

      int chunkSize = (int)strtol(chunkSizeLine.c_str(), NULL, 16);

      if (chunkSize <= 0) {
        client.readStringUntil('\n');
        break;
      }

      int remaining = chunkSize;
      while (remaining > 0) {
        while (!client.available() && client.connected()) {
          delay(1);
        }

        int toRead = remaining;
        if (toRead > 128) toRead = 128;

        char buffer[129];
        int bytesRead = client.readBytes(buffer, toRead);
        buffer[bytesRead] = '\0';

        responseBody += buffer;
        remaining -= bytesRead;
      }

      client.readStringUntil('\n');
    }
  } else if (contentLength > 0) {
    int remaining = contentLength;
    while (remaining > 0) {
      while (!client.available() && client.connected()) {
        delay(1);
      }

      int toRead = remaining;
      if (toRead > 128) toRead = 128;

      char buffer[129];
      int bytesRead = client.readBytes(buffer, toRead);
      buffer[bytesRead] = '\0';

      responseBody += buffer;
      remaining -= bytesRead;
    }
  } else {
    unsigned long lastData = millis();
    while (millis() - lastData < 3000) {
      while (client.available()) {
        responseBody += client.readString();
        lastData = millis();
      }
      delay(10);
    }
  }

  if (responseBody.length() == 0) {
    Serial.println("Gemini Body leer");
    return "";
  }

  JsonDocument responseDoc;
  DeserializationError error = deserializeJson(responseDoc, responseBody);

  if (error) {
    Serial.print("Gemini JSON Fehler: ");
    Serial.println(error.c_str());
    Serial.println(responseBody);
    return "";
  }

  const char* finishReason = responseDoc["candidates"][0]["finishReason"];
  Serial.print("finishReason: ");
  Serial.println(finishReason ? finishReason : "null");

  const char* answer = responseDoc["candidates"][0]["content"]["parts"][0]["text"];
  if (!answer) {
    Serial.println("Keine Gemini-Antwort gefunden");
    return "";
  }

  return String(answer);
}

// Fuehrt den kompletten KI Ablauf einmal durch.
void starteSprachAblauf() {
  Serial.println("Starte Sprachablauf...");

  zeigeNachricht("HEY <3", "ERZAEHL!");

  if (!recordWav()) {
    Serial.println("Aufnahme fehlgeschlagen");
    return;
  }

  String fileUri;
  String mimeType;
  if (!uploadFileToGemini(fileUri, mimeType)) {
    Serial.println("Gemini File Upload fehlgeschlagen");
    zeigeNachricht("UPS", "FEHLER");
    return;
  }

  Serial.print("Gemini file_uri: ");
  Serial.println(fileUri);

  zeigeNachricht("ICH DENKE", "NACH...");

  String antwort = frageGeminiMitDatei(fileUri, mimeType);
  antwort.trim();

  if (antwort == "") {
    Serial.println("Gemini-Anfrage fehlgeschlagen");
    zeigeNachricht("KEINE", "ANTWORT");
    return;
  }

  Serial.println();
  Serial.println("===== GEMINI-ANTWORT =====");
  Serial.println(antwort);
  Serial.println("==========================");

  zeigeAntwort(antwort);
}
