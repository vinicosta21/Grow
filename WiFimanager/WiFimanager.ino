#include <WiFiManager.h>
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  WiFiManager wm;
  wm.resetSettings();
  bool res;
  res = wm.autoConnect("Grow");

  if(!res){
    Serial.println("Failed to connect");
    ESP.restart();
  }

  else {
    Serial.println("Conectou! :)");
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
