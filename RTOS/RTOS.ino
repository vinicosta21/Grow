#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#include <HTTPClient.h>
#include <WiFiManager.h>
#include "time.h"
#include "bomba.h"
#include "luz.h"
#include "sensor_umidade.h"
#include "BH1750.h"

/***********************************************************************
 Componentes
 ***********************************************************************/
SensorUmidade umd;
BH1750 lum;
BombaAgua bag;
Luz luz;
WiFiManager wm;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -10800;
const int   daylightOffset_sec = 0;

int delta_t_luz = 16;
float min_lux = 2000;
int seco = 2000;
char buffer[40];


// define 2 tasks para Request e Interrupcoes
void TaskRequest( void *pvParameters );
void TaskLuz( void *pvParameters );
void TaskIrrigar( void *pvParameters );


// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
  Wire.begin();
  lum.begin();

  
//  wm.resetSettings();
  bool connec = wm.autoConnect("Grow"); // Acessar 192.168.4.1
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Now set up two tasks to run independently.
  xTaskCreatePinnedToCore(
    TaskRequest
    ,  "Requests"   // A name just for humans
    ,  8192  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL 
    ,  1);

  xTaskCreate(
    TaskLuz
    ,  "Luz"
    ,  1024  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL );

    xTaskCreate(
    TaskIrrigar
    ,  "Irrigar"
    ,  1024  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL );


  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskRequest(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
    
  If you want to know what pin the on-board LED is connected to on your ESP32 model, check
  the Technical Specs of your board.
*/

  

  for (;;) // A Task shall never return or exit.
  {
    HTTPClient client;
    client.begin("https://young-harbor-35740.herokuapp.com/dados");
    int umidade = umd.medirUmidade();
    int lux = (int)lum.readLightLevel();
    sprintf(buffer, "vinicius,%d,%d\0",lux,umidade);
    Serial.println(buffer);
    int code = client.POST(buffer);
    if (code > 0){
      String payload = client.getString();
      Serial.println("\nStatuscode: " + String(code));
      Serial.println(payload);
      int str_len = payload.length() + 1;
      char char_array[str_len];
      payload.toCharArray(char_array, str_len);
      char *token;
      String t_aux, umd_aux, luz_aux;
      t_aux = (String)strtok(char_array, ",");
      delta_t_luz = t_aux.toInt();
      Serial.println(delta_t_luz);
      luz_aux = (String)strtok(NULL, ",");
      min_lux = (float)luz_aux.toInt();
      Serial.println(min_lux);
      umd_aux = (String)strtok(NULL, ",");
      seco = umd_aux.toInt();
      Serial.println(seco);
      
    }
    else{
      Serial.println("Erro no request");
    }
    client.end();
    vTaskDelay(5000);
  }
}

void TaskLuz(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  
/*
  AnalogReadSerial
  Reads an analog input on pin A3, prints the result to the serial monitor.
  Graphical representation is available using serial plotter (Tools > Serial Plotter menu)
  Attach the center pin of a potentiometer to pin A3, and the outside pins to +5V and ground.

  This example code is in the public domain.
*/

  for (;;)
  {
    float lux = lum.readLightLevel();
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    if (lux < min_lux && !(timeinfo.tm_hour < 12 - delta_t_luz / 2 || timeinfo.tm_hour > 12 + delta_t_luz / 2)){
      luz.ligarLuz();
    }
    else{
      luz.desligarLuz();
    }
    vTaskDelay(5000);
  }
}

void TaskIrrigar(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  
/*
  AnalogReadSerial
  Reads an analog input on pin A3, prints the result to the serial monitor.
  Graphical representation is available using serial plotter (Tools > Serial Plotter menu)
  Attach the center pin of a potentiometer to pin A3, and the outside pins to +5V and ground.

  This example code is in the public domain.
*/

  for (;;)
  {
    int umidade = umd.medirUmidade();
    if (umidade > seco){
      bag.ligarBomba();
      delay(5000);
      bag.desligarBomba();
      vTaskDelay(5000);
    }
    else {
      vTaskDelay(300000);
    }
  }
}
