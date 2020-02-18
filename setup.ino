void setup() 
{  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RESET_WIFI, INPUT_PULLUP);
  pinMode(MODE_BUTTON, INPUT_PULLUP);

  Serial.begin(SERIAL_BAUD);
  while(!Serial) {}
    
  Serial.println("Device '" + deviceName + "' is starting...");

  if (WiFi.SSID() != "") {
    Serial.print("Current Saved WiFi SSID: ");
    Serial.println(WiFi.SSID());

    // reset wifi by RESET_WIFI pin to GROUND
    int resetCycle = 0;
    ticker1.attach_ms(35, tickInternal);
    while (resetCycle < 50) {
      MODE_RESET_WIFI = digitalRead(RESET_WIFI);
      if (MODE_RESET_WIFI == LOW) {
        resetWiFiSettings();
        break;
      }
      resetCycle++;
      delay(36);
    }
    // end reset wifi
  } else {
    Serial.println("We dont have saved WiFi settings, need configure");
  }
  
  WiFi.hostname(deviceName);

  Wire.begin();
  
  if (!setupWiFiManager()) {
    Serial.println("failed to connect and hit timeout");
    delay(30000);
    ESP.restart();
  } else {
    Serial.println("WiFi network connected");
    NO_INTERNET = false;

    if (SPIFFS.begin()) {
      Serial.println(F("SPIFFS was mounted"));
    } else {
      Serial.println(F("Error while mounting SPIFFS"));
    }
  
    int customInterval = readCfgFile("interval").toInt();
    if (customInterval > 5) {
      MON_INTERVAL = customInterval * 1000;
    }
  
    String customSsl = readCfgFile("ssl");
    if (customSsl) {
      OsMoSSLFingerprint = customSsl;
    }
  
    TOKEN = readCfgFile("token");

    Serial.println("Syncing time...");
    int syncSecs = 0;
    configTime(0, 0, "pool.ntp.org");  
    setenv("TZ", "GMT+0", 0);
    while(time(nullptr) <= 100000) {
      if (syncSecs > 15) {
        return ESP.restart();
      }
      
      Serial.print(" .");
      syncSecs++;
      delay(1000);
    }
    Serial.println();

    StaticJsonDocument<1024> jb;
    String postData = 
      "token=" + TOKEN + "&" +
      "revision=" + String(DEVICE_REVISION) + "&" +
      "model=" + String(DEVICE_MODEL) + "&" +
      "firmware=" + String(DEVICE_FIRMWARE) + "&"
      "ip=" + WiFi.localIP().toString() + "&" +
      "mac=" + String(WiFi.macAddress()) + "&" +
      "ssid=" + String(WiFi.SSID()) + "&" +
      "rssi=" + String(WiFi.RSSI()) + "&" +
      "vcc=" + String(ESP.getVcc());
    Serial.println(postData);

    const size_t capacity = JSON_OBJECT_SIZE(10) + JSON_ARRAY_SIZE(10) + 60;
    DynamicJsonDocument doc(capacity);

    HTTPClient http;
    http.begin("http://iot.osmo.mobi/device");
    http.setUserAgent(deviceName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpCode = http.POST(postData);
    if (httpCode != HTTP_CODE_OK && !CHIP_TEST) {
        Serial.println("Error init device from OsMo.mobi");
        delay(20000);
        return ESP.restart();
    }
    Serial.println("get device config and set env, result: " + String(httpCode));
    if (httpCode == HTTP_CODE_OK) {
      NO_SERVER = false;
    }
    
    String payload = http.getString();
    deserializeJson(doc, payload);
    http.end();

    Serial.print(F("Interval: "));
    Serial.println(doc["interval"].as<int>());
    if (doc["interval"].as<int>() > 4) {
      int MON_INTERVAL_NEW = doc["interval"].as<int>() * 1000;
      if (MON_INTERVAL != MON_INTERVAL_NEW) {
        MON_INTERVAL = MON_INTERVAL_NEW;
        writeCfgFile("interval", doc["interval"].as<String>());
      }
    }

    Serial.print("SHA-1 FingerPrint for SSL KEY: ");
    Serial.println(doc["tlsFinger"].as<String>());
    if (OsMoSSLFingerprint != doc["tlsFinger"].as<String>()) {
      OsMoSSLFingerprint = doc["tlsFinger"].as<String>();
      writeCfgFile("ssl", OsMoSSLFingerprint);
      Serial.print("tlsFinger was updated in SPIFFS");
    }

    Serial.print("TOKEN: ");
    Serial.println(doc["token"].as<String>());
    if (TOKEN != doc["token"].as<String>()) {
      TOKEN = doc["token"].as<String>();
      writeCfgFile("token", TOKEN);
      Serial.print("Token was updated in SPIFFS");
    }

    tickOffAll();
    checkFirmwareUpdate();
  }

//  Serial.println("SHT31 test");
//  if (!sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
//    Serial.println("Couldn't find SHT31");
//    while(1) delay(1);
//  }

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  display.display();
  delay(1000);  

  display.clearDisplay();
  display.display();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Hi!");
  display.println("OsMo.mobi");
  display.setCursor(0,0);
  display.display();

  delay(1000);

  tickOffAll();

  digitalWrite(BUILTIN_LED, HIGH);
}
