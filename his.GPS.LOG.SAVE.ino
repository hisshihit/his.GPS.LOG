// M5Stack ----------------------------------------------------------------
#include <M5Stack.h>
#include "M5StackUpdater.h"
// BLE & Cocoa ------------------------------------------------------------
#include <BLEDevice.h>
// ENV2 -------------------------------------------------------------------
/* add library Adafruit_BMP280 & Adafruit_SHT31 from library manage */
#include <Adafruit_Sensor.h>
#include <Adafruit_SHT31.h>
#include <Wire.h> //The SHT31 uses I2C comunication.
#include <Adafruit_BMP280.h>
// GPS --------------------------------------------------------------------
#include <TinyGPS++.h>
// SD ---------------------------------------------------------------------
#include <SPI.h>
#include <SD.h>
// BLE & Cocoa ------------------------------------------------------------
int scanTime = 5;
BLEScan* pBLEScan;
const int chipSelect = 4;
bool onBeep = true;

//接触確認アプリのUUID
const char* cocoaUUID = "0000fd6f-0000-1000-8000-00805f9b34fb";
int cocoaCnt = 0; //time out in seconds

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      if (advertisedDevice.haveServiceUUID()) {
        if (strncmp(advertisedDevice.getServiceUUID().toString().c_str(), cocoaUUID, 36) == 0) {
          cocoaCnt++;
          if (onBeep) {
            M5.Speaker.beep();
            delay(10);
            M5.Speaker.mute();
          }
          M5.Lcd.setCursor(0, 110);
          M5.Lcd.setTextSize(2);
          M5.Lcd.setTextColor(GREEN, BLACK);
          M5.Lcd.printf("%d ", cocoaCnt);
          M5.Lcd.setTextSize(1);
        }
      }
      M5.Lcd.println(advertisedDevice.toString().c_str());
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(WHITE, BLACK);
    }
};

void setupBLE() {
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}
// ENV2 -------------------------------------------------------------------
Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_BMP280 bme;

bool hasSHT31 = false;
bool hasBMP280 = false;
float tmp = 100;
float hum = 0;
float pressure = 0;

void setupENV2() {
  //ENV2 sensor on I2C setup
  Wire.begin();
  Serial.println(F("ENV.2 Chk"));

  if (sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    hasSHT31 = true;
    Serial.println("SHT31 Ok");

    Serial.print("Heater: ");
    if (sht31.isHeaterEnabled())
      Serial.println("Ena");
    else
      Serial.println("Dis");
  }

  if (bme.begin(0x76)) {
    hasBMP280 = true;
    Serial.println("BMP280 Ok");
  }
}
// GPS --------------------------------------------------------------------
//bool hasRTC = false;
bool hasGPS = false;
TinyGPSPlus gps; // The TinyGPS++ object
HardwareSerial hsGps(2);// The serial connection to the GPS device
static const uint32_t GPSBaud = 9600;

void setupGPS() {
  hsGps.begin(GPSBaud);
  delay(500);

  if (hsGps.available() > 0) {
    hasGPS = true;
    Serial.println("GPS Ok");
  }
}
// SD ---------------------------------------------------------------------
// log file name
const char* logfile = "/GPSlog.jsn";
// Inside -----------------------------------------------------------------
unsigned char bright = 0x03;
unsigned char brightPitch = 0x10;
//文字列
const char* logJSON = "{ \"class\": \"TPV\" ,\
\"mode\": 3 ,\
\"time\": \"%d-%02d-%02dT%02d:%02d:%02dZ\" ,\
\"lat\": %lf ,\
\"lon\": %lf ,\
\"alt\": %.1f ,\
\"track\": %lf ,\
\"speed\": %.1f ,\
\"ble\": %d ,\
\"cocoa\": %d ,\
\"temp\": %.1f ,\
\"humidity\": %.1f ,\
\"pressure\": %.1f }";

// ------------------------------------------------------------------------
void setup() {

  // Initialize the M5Stack
  M5.begin();
  M5.Power.begin();
  M5.Lcd.setBrightness(bright);
  M5.Lcd.setTextSize(1);
  M5.Lcd.println("Hello!, COCOA Scan");

  Serial.begin(115200);

  setupBLE();
  setupENV2();
  setupGPS();

  if (digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }

}

void loop() {
  // print all found BLE devices
  M5.Lcd.setTextSize(1);

  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);

  // print counts of BLE devices
  int sumdev = foundDevices.getCount();

  // print env2 data
  if (hasSHT31) {
    tmp = sht31.readTemperature();
    hum = sht31.readHumidity();
  }
  if (hasBMP280) {
    pressure = bme.readPressure() / 100;
    // hPa = Pa / 100;
  }

  while (hsGps.available() > 0) {
    gps.encode(hsGps.read());
  }

  Serial.printf(logJSON, gps.date.year(), gps.date.month(), gps.date.day(),
                gps.time.hour(), gps.time.minute(), gps.time.second(),
                gps.location.lat(), gps.location.lng(), gps.altitude.meters(), gps.course.deg(),
                gps.speed.mps(), sumdev, cocoaCnt, tmp, hum, pressure);
  Serial.println();

  // SDカードへの書き込み処理（ファイル追加モード）
  // SD.beginはM5.begin内で処理されているので不要
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dfile = SD.open(logfile, FILE_APPEND);

  // if the file is available, write to it:
  if (dfile) {
    dfile.printf(logJSON, gps.date.year(), gps.date.month(), gps.date.day(),
                 gps.time.hour(), gps.time.minute(), gps.time.second(),
                 gps.location.lat(), gps.location.lng(), gps.altitude.meters(), gps.course.deg(),
                 gps.speed.mps(), sumdev, cocoaCnt, tmp, hum, pressure);
    dfile.println();
  }

  // clear screen and set cursor to the top
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.printf(logJSON, gps.date.year(), gps.date.month(), gps.date.day(),
                gps.time.hour(), gps.time.minute(), gps.time.second(),
                gps.location.lat(), gps.location.lng(), gps.altitude.meters(), gps.course.deg(),
                gps.speed.mps(), sumdev, cocoaCnt, tmp, hum, pressure);
  M5.Lcd.println();

  //Button controll
  M5.Lcd.setTextSize(3);
  M5.Lcd.println();
  M5.Lcd.setTextColor(BLUE, WHITE);
  M5.Lcd.println("A: SDUpdater");
  M5.Lcd.println("B: Beep on/off");
  M5.Lcd.println("C: Brightness");

  int timer = 100;
  while (timer--) {
    if (M5.BtnA.wasReleased()) { //AボタンでSDUpdater
      updateFromFS(SD);
      ESP.restart();
    } else if (M5.BtnB.wasReleased()) { //Bボタンでbeepをon/off切り替える
      onBeep = !onBeep;
    } else if (M5.BtnC.wasReleased()) { //Cボタンで輝度を変更
      bright += brightPitch;
      M5.Lcd.setBrightness(bright);
    }
    delay(50);
    M5.update(); // update button state
  }

  //init for next loop
  dfile.close();
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  cocoaCnt = 0;

  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setCursor(0, 110);
  M5.Lcd.setTextSize(1);
}

// Arranged and written by 柴田(ひ)
/* BLE
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/
/* Cocoa
   Thanks to https://gist.github.com/ksasao/0da6437d3eac9b2dbd675b6fee5d1117
   by https://gist.github.com/ksasao
*/
/* GPS
   This sample sketch demonstrates the normal use of a TinyGPS++ (TinyGPSPlus) object.
   by Mikal Hart
*/
/* SD
   SD card wrie routine
   https://raspberrypi.mongonta.com/howto-write-csv-to-sdcard-on-m5stack/
*/
