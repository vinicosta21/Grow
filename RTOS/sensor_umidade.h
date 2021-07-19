#include <Arduino.h>
#ifndef VCC_SENS_H2O
#define VCC_SENS_H2O 32
#endif

#ifndef SENS_H2O
#define SENS_H2O 34

class SensorUmidade
{
  public:
	  SensorUmidade();
	  int medirUmidade();
};

#endif
