#include "Bomba.h"

BombaAgua::BombaAgua(){
	pinMode(RELE_2, OUTPUT);
}

void BombaAgua::ligarBomba(){
	digitalWrite(RELE_2, HIGH);
}

void BombaAgua::desligarBomba(){
	digitalWrite(RELE_2, LOW);
}