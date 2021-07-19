#include "sensor_umidade.h"

SensorUmidade::SensorUmidade() {
	pinMode(VCC_SENS_H2O, OUTPUT);
	pinMode(SENS_H2O, INPUT);
}

int SensorUmidade::medirUmidade() {
	digitalWrite(VCC_SENS_H2O, HIGH);
	int umidade = analogRead(SENS_H2O);
	digitalWrite(VCC_SENS_H2O, LOW);
	return umidade;
}
