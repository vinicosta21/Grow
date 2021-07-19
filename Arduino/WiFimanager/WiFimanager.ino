#include <BH1750.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
bool connec;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  WiFiManager wm;
  wm.resetSettings();
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
    client.begin("https://young-harbor-35740.herokuapp.com/dados");
    int code = client.POST("vinicius,88,37");
    if (code > 0){
      String payload = client.getString();
      Serial.println("\nStatuscode: " + String(code));
      Serial.println(payload);
    }
    else{
      Serial.println("Erro no request");
    }
    client.end();
  }
  else {
    Serial.println("Sem conexão :(");
  }
  
  delay(5*60000);
}
