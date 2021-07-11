#include <BH1750.h>
#include <Arduino_JSON.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <Wifi.h>
bool connec;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  WiFiManager wm;
//  wm.resetSettings();
  connec = wm.autoConnect("Grow");

  if(!connec){
    Serial.println("Failed to connect");
    ESP.restart();
  }

  else {
    Serial.println("Conectou! :)");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if (connec) {
    HTTPClient client;
    client.begin("https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json");
    int code = client.GET();    
    if (code > 0){
      String payload = client.getString();
      Serial.println("\nStatuscode: " + String(code));
      Serial.println(payload);
    }
    else{
      Serial.println("Erro no request");
    }
    
  }
  else {
    Serial.println("Sem conexão :(");
  }

  delay(10000);
}
