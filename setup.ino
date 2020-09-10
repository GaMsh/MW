void setup() 
{  
  pinMode(LED_BUILTIN, OUTPUT);
//  pinMode(LED_EXTERNAL, OUTPUT);
  pinMode(RESET_WIFI, INPUT_PULLUP);

  Serial.begin(SERIAL_BAUD);
  while(!Serial) {}
    
  Serial.println("Device '" + deviceName + "' is starting...");
  Serial.println("Voltage: " + String(ESP.getVcc()));

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
    Serial.println("WiFi network connected (" + String(WiFi.RSSI()) + ")");
    NO_INTERNET = false;
    checkFirmwareUpdate();

    if (LittleFS.begin()) {
      Serial.println(F("LittleFS was mounted"));
    } else {
      Serial.println(F("Error while mounting LittleFS"));
    }

    ///// Config 
    String customSsl = readCfgFile("ssl");
    if (customSsl) {
      OsMoSSLFingerprint = customSsl;
    }

    tickOffAll();

    getDeviceConfiguration();
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
