#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

//needed for local file system SFIFFS working
#include <FS.h>
#include <ArduinoJson.h>

//needed for library WiFiManager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

//needed for I2C
#include <Wire.h>

//needed for sensor SHT31
#include <Adafruit_SHT31.h>
Adafruit_SHT31 sht31 = Adafruit_SHT31();

//needed for display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//needed for LED status
#include <Ticker.h>

// // // это был длииинный список библиотек для запуска этой штуки :)))

Ticker ticker1;

#define SERIAL_BAUD 115200 // скорость Serial порта, менять нет надобности
#define CHIP_TEST 0 // если нужно протестировать плату (отключает перезагрузку), задайте 1
#define NO_AUTO_UPDATE 1 // если нужно собрать свою прошивку и не получить перезатирание через OTA, задайте 1

int MON_INTERVAL = 20000; // интервал опроса датчиков по умолчанию
int REBOOT_INTERVAL = 6 * 60 * 60000; // интервал принудительной перезагрузки устройства, мы не перезагружаемся, если нет сети

boolean NO_INTERNET = true; // флаг состояния, поднимается если отвалилась wifi сеть
boolean NO_SERVER = true; // флаг состояния, поднимается если отвалился сервер

int MODE_RESET_WIFI = 0; // флаг состояния, поднимается если отвалился сервер

const char* DEVICE_MODEL = "GaM_WM1";
const char* DEVICE_REVISION = "july"; 
const char* DEVICE_FIRMWARE = "1.0.0";

const int RESET_WIFI = D3;
const int MODE_BUTTON = D5;

unsigned long previousMillis = MON_INTERVAL * -2; //Чтобы начинать отправлять данные сразу после запуска
unsigned long previousMillisReboot = 0;

String deviceName = String(DEVICE_MODEL) + "_" + String(DEVICE_FIRMWARE);

String OsMoSSLFingerprint = ""; //69 3B 2D 26 B2 A7 96 5E 10 E4 2F 84 63 56 CE ED E2 EC DA A3
String TOKEN = "";

int bytesWriten = 0;
int currentMode = 0;


// WifiManager callback
void configModeCallback(WiFiManager *myWiFiManager) 
{
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
  ticker1.attach_ms(500, tickInternal);
}
