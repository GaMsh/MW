void loop() 
{
  unsigned long currentMillis = millis();

  if (!CHIP_TEST) {
    if (currentMillis - previousMillisReboot > REBOOT_INTERVAL) {
      Serial.println("It`s time to reboot");
      if (!NO_INTERNET && !NO_SERVER) {
        ESP.restart();
      } else {
        Serial.println("But it`s impossible, no internet connection");
      }
    }
  }

  if (currentMillis - previousMillis >= MON_INTERVAL) {
    previousMillis = currentMillis;
        
    ticker1.detach();
    
    digitalWrite(LED_BUILTIN, LOW);
    
    mainProcess();

    digitalWrite(LED_BUILTIN, HIGH);
  }
}
