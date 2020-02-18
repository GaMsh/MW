void checkFirmwareUpdate() {
//  if (!NO_AUTO_UPDATE && !NO_INTERNET) {
//    t_httpUpdate_return ret = ESPhttpUpdate.update("http://tw.gamsh.ru", DEVICE_FIRMWARE);
//    
//    switch (ret) {
//      case HTTP_UPDATE_FAILED:
//        Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
//        break;
//      case HTTP_UPDATE_NO_UPDATES:
//        Serial.println("[update] Update no Update.");
//        break;
//      case HTTP_UPDATE_OK:
//        Serial.println("[update] Update ok.");
//        ESP.restart();
//        break;
//    }
//  }
}

void mainProcess() {
  if (WiFi.status() == WL_CONNECTED) {
    callToServer();
  }
}

boolean callToServer() {
//  if (NO_INTERNET) {
//    NO_INTERNET = false;
//
//    return  display.setCursor(0, 0); bufferReadAndSend();
//  }

  HTTPClient http; 
  http.begin("http://iot.osmo.mobi/export/iot_set_7.json?tz=Europe/Moscow"); //, OsMoSSLFingerprint);
  http.setUserAgent(deviceName);

  int httpCode = http.GET();
  Serial.println("Getting from server...");
  if (httpCode != HTTP_CODE_OK) {
    NO_SERVER = true;
    return false;
  }
  NO_SERVER = false;
  String payload = http.getString();
  StaticJsonDocument<2048> doc;
  deserializeJson(doc, payload, DeserializationOption::NestingLimit(6));
  http.end();

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(doc["time"]["update"]["time"].as<String>());
  display.print(doc["data"]["summary"]["paramsRaw"]["T:"].as<float>());
  display.print(" -- ");
  display.println(doc["data"]["deviceCountFact"].as<int>());
  display.println(doc["data"]["summary"]["paramsRaw"]["H:"].as<float>());
  display.println(doc["data"]["summary"]["paramsRaw"]["P:"].as<float>());
  display.display();

  return true;
}
