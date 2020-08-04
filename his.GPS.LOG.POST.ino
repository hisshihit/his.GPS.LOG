// M5Stack ----------------------------------------------------------------
#include <M5Stack.h>
#include "M5StackUpdater.h"
// HTTP -------------------------------------------------------------------
#include <HTTPClient.h>
// JSON -------------------------------------------------------------------
#include <ArduinoJson.h>
// SD ---------------------------------------------------------------------
#include <SPI.h>
#include <SD.h>
// WiFi -------------------------------------------------------------------
#include <WiFi.h>

// CONFIG -----------------------------------------------------------------
struct Config {
  char logfile[12];
  int  buf;
  char ssid[64];
  char pass[64];
  char url[256];
};

Config config;

const char *cnfFileName = "/GPSlog.cnf"; //環境毎に設定

File logFile;

void loadConfiguration(const char *filename, Config &config) {
  File file = SD.open(cnfFileName);
  StaticJsonDocument<512> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("ERROR while reading config file. using default conf"));

  // Copy values from the JsonDocument to the Config
  strlcpy(config.logfile,
          doc["logfile"] | "/GPSlog.jsn",  //環境毎に設定
          sizeof(config.logfile));
  Serial.println("log file name: " + String(config.logfile));
  config.buf = doc["buf"] | 1024;         //環境毎に設定
  Serial.println("JSON line buffer size: " + String(config.buf));
  strlcpy(config.ssid,
          doc["ssid"] | "SSID-open",  //環境毎に設定
          sizeof(config.ssid));
  Serial.println("SSID: " + String(config.ssid));
  strlcpy(config.pass,
          doc["pass"] | "passowrd",  //環境毎に設定
          sizeof(config.pass));
  Serial.println("pass: " + String(config.pass));
  strlcpy(config.url,
          doc["url"] | "http://example.com:3000/postjson",  //環境毎に設定
          sizeof(config.url));
  Serial.println("http POST url: " + String(config.url));

  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
}
// HTTP -------------------------------------------------------------------
bool dohttpPOST(String logstring)
{
  bool b = false;
  HTTPClient http;
  http.begin(config.url);
  http.addHeader("Content-Type", "application/json");
  int status_code = http.POST(logstring);
  Serial.printf("status_code=%d\r\n", status_code);
  if (status_code == 200) {
    String json_response = http.getString();
    Serial.println("respose->"); Serial.println(json_response);
    b = true;
  }
  http.end();
  return b;
}
// WiFi -------------------------------------------------------------------
void connectWiFi() {
  WiFi.mode(WIFI_STA);  //STAモード（子機）として使用
  WiFi.disconnect();    //Wi-Fi切断

  WiFi.begin(config.ssid , config.pass); //環境毎に設定
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("trying WiFi Connection...");
    delay(1000);
  }
  Serial.println("WiFi Connected.");
}

// Inside -----------------------------------------------------------------
void setup() {
  M5.begin();
  M5.Power.begin();
  M5.Lcd.println("Hello!, JSON http.post");

  Serial.begin(115200);
  loadConfiguration(cnfFileName, config);

  //SD -------------------------------------------
  logFile = SD.open(String(config.logfile));
  if (logFile) {
    Serial.println("log file found");
  } else {
    Serial.println("error: opening " + String(config.logfile));
  }

  //WiFI -------------------------------------------
  connectWiFi();

  //HTTP -------------------------------------------
  int pt = 0;
  char readbuf[config.buf] = "";
  while (logFile.available()) {
    readbuf[pt] = logFile.read();
    if (readbuf[pt] == 0x0A) {
      readbuf[pt + 1] = 0;
      String s = readbuf;
      Serial.println("String length=" + String(s.length()));
      Serial.println(s);
      if (dohttpPOST(s)) {
        Serial.println("post OK");
      } else {
        Serial.println("post ERROR");
      }
      pt = 0;
      char readbuf[config.buf] = "";
      delay(50);
    } else {
      pt++;
    }
  }
}

void loop() {
  M5.update();
  M5.Lcd.setTextSize(3);
  M5.Lcd.println();
  M5.Lcd.setTextColor(BLUE, WHITE);
  M5.Lcd.println("log file http.POST done.");
  M5.Lcd.println("A: SDUpdater");
  while (1) {
    if (M5.BtnA.wasReleased()) { //AボタンでSDUpdater
      updateFromFS(SD);
      ESP.restart();
    }
    delay(100);
    M5.update(); // update button state
  }
}
// Arranged and written by 柴田(ひ)
