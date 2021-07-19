#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#include <Wire.h>
#include<WiFi.h>
#include <BH1750.h>
//#include "sensor_umidade.h"
#include "time.h"

BH1750 lightMeter;

// define two tasks for Blink & AnalogRead
void TaskBlink( void *pvParameters );
void TaskAnalogReadA3( void *pvParameters );

const long  gmtOffset_sec = -10800;
const char* ntpServer = "pool.ntp.org";
const int daylightOffset_sec = 0;

const char* ssid       = "costa";
const char* password   = "anaedu13071986";

int LUM_MIN = 2000;
int HORA_MIN = 6;
int HORA_MAX = 18;


// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  
  
  // Now set up two tasks to run independently.
  xTaskCreatePinnedToCore(
    TaskIluminar
    ,  "Iluminacao"   // A name just for humans
    ,  1024  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskIrrigacao
    ,  "Irrigacao"
    ,  1024  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskIluminar(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
    
  If you want to know what pin the on-board LED is connected to on your ESP32 model, check
  the Technical Specs of your board.
*/

  // initialize digital LED_BUILTIN on pin 13 as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  for (;;) // A Task shall never return or exit.
  {
    Serial.println("Rodou");
    vTaskDelay(10000);
  }
}

void TaskIrrigacao(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  
/*
  AnalogReadSerial
  Reads an analog input on pin A3, prints the result to the serial monitor.
  Graphical representation is available using serial plotter (Tools > Serial Plotter menu)
  Attach the center pin of a potentiometer to pin A3, and the outside pins to +5V and ground.

  This example code is in the public domain.
*/
  Wire.begin();
  lightMeter.begin();
  const int RELE_1 = 27;
  pinMode(RELE_1, OUTPUT);
  digitalWrite(RELE_1, HIGH);
  
  for (;;)
  {
    // read the input on analog pin A3:
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    float lux = lightMeter.readLightLevel();
    // print out the value you read:
    Serial.println(lux);
    if ((timeinfo.tm_hour > HORA_MIN || timeinfo.tm_hour < HORA_MAX) && lux < LUM_MIN) {
      digitalWrite(RELE_1, HIGH);
    }
    else {
      digitalWrite(RELE_1, LOW);
    }
    vTaskDelay(1000);  // one tick delay (15ms) in between reads for stability
  }
}
