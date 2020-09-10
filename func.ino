// core functions
bool getDeviceConfiguration() {
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
  http.begin(OSMO_HTTP_SERVER_DEVICE);
  http.setUserAgent(deviceName);
  http.setTimeout(30000);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.POST(postData);
  if (httpCode != HTTP_CODE_OK && !CHIP_TEST) {
    ticker1.attach_ms(200, tickInternal);
//    ticker2.attach_ms(500, tickExternal, MAIN_MODE_OFFLINE);
    
    Serial.println("Error init device from OsMo.mobi");
    delay(15000);
    ESP.restart();
    return false;
  }
  Serial.println("get device config and set env, result: " + String(httpCode));
  if (httpCode == HTTP_CODE_OK) {
    NO_SERVER = false;
  } else {
    return false;
  }
  
  String payload = http.getString();
  deserializeJson(doc, payload);
  http.end();

  int mytime = doc["time"].as<int>();
  Serial.println(mytime);

  struct timeval tv;
  tv.tv_sec = mytime;
  settimeofday(&tv, NULL);

  if (OsMoSSLFingerprint != doc["tlsFinger"].as<String>()) {
    OsMoSSLFingerprint = doc["tlsFinger"].as<String>();
    writeCfgFile("ssl", OsMoSSLFingerprint);
    Serial.println("tlsFinger was updated in store");
  }

  if (TOKEN != doc["token"].as<String>()) {
    TOKEN = doc["token"].as<String>();
    writeCfgFile("token", TOKEN);
    Serial.println("Token was updated in store");
  }

  return true;
}

// LEDs functions
void tickInternal()
{
  int stateBack = digitalRead(BUILTIN_LED);
  digitalWrite(BUILTIN_LED, !stateBack);
}

void tickOffAll()
{
  ticker1.detach();
}
