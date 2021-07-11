const int port_umid1 = 34;
const int vcc_umid1 = 32;
int umid1 = 0;
int s_delay = 5;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  pinMode(vcc_umid1, OUTPUT);
  pinMode(port_umid1, INPUT);
  Serial.println("Comecando... ");
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(vcc_umid1, HIGH);
  umid1 = analogRead(port_umid1);
  digitalWrite(vcc_umid1, LOW);
  Serial.println("\nValor: ");
  Serial.print(umid1);
  delay(s_delay * 1000);
}
