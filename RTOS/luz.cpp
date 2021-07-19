#include "luz.h"

Luz::Luz(){
	pinMode(RELE_1, OUTPUT);
}

void Luz::ligarLuz(){
	digitalWrite(RELE_1, HIGH);
}

void Luz::desligarLuz(){
	digitalWrite(RELE_1, LOW);
}