/*
    ALARME RESIDENCIAL
*/
#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <semphr.h>

#include "bomba.h"
#include "luz.h"
#include "sensor_umidade.h"
#include "BH1750.h"

#define TASK_INTERVAL1 5000
#define TASK_INTERVAL2 5000
#define TASK_INTERVAL3 300000

/***********************************************************************
 Componentes
 ***********************************************************************/
SensorUmidade umd;
BH1750 lum;
HTTPClient http;
BombaAgua bag;
Iluminacao luz;




/***********************************************************************
 Estaticos
 ***********************************************************************/
int codigoEvento = NENHUM_EVENTO;
int eventoInterno = NENHUM_EVENTO;
int estado = ESPERA;
int codigoAcao;
int acao_matrizTransicaoEstados[NUM_ESTADOS][NUM_EVENTOS];
int proximo_estado_matrizTransicaoEstados[NUM_ESTADOS][NUM_EVENTOS];

/***********************************************************************
 FreeRTOS
 ***********************************************************************/
void taskMaqEstados(void *pvParameters);
void taskObterEvento(void *pvParameters);
QueueHandle_t xQueue;
SemaphoreHandle_t xBinarySemaphore;
TaskHandle_t xTaskMaqEstados, xTaskObterEvento, xTaskBipBip;

/************************************************************************
 executarAcao
 Executa uma acao
 Parametros de entrada:
    (int) codigo da acao a ser executada
 Retorno: (int) codigo do evento interno ou NENHUM_EVENTO
*************************************************************************/
int executarAcao(int codigoAcao)
{
    int retval;

    retval = NENHUM_EVENTO;
    if (codigoAcao == NENHUMA_ACAO)
        return retval;

    switch(codigoAcao)
    {
    case A01:
        tmr.iniciar(true);
        break;
    case A02:
        sne.bip();
        com.notificar("Alarme em alerta");
        tmr.iniciar(false);
        break;
    case A03:
        com.notificar("Alarme desacionado");
        tmr.iniciar(false);
        break;
    case A04:
        com.notificar("Alarme desacionado");
        break;
    case A05:
        tmr.iniciar(true);
        break;
    case A06:
        sne.acionar(true);
        if( xSemaphoreGive(xBinarySemaphore) != pdPASS )
          Serial.println("Erro ao enviar para semáforo");
        com.notificar("Invasao");
        tmr.iniciar(false);
        break;
    case A07:
        com.notificar("Alarme desacionado");
        tmr.iniciar(false);
        sne.acionar(false);
        break;
    } // switch

    return retval;
} // executarAcao

/************************************************************************
 iniciaMaquina de Estados
 Carrega a maquina de estados
 Parametros de entrada: nenhum
 Retorno: nenhum
*************************************************************************/
void iniciaMaquinaEstados()
{
  int i;
  int j;

  for (i=0; i < NUM_ESTADOS; i++) {
    for (j=0; j < NUM_EVENTOS; j++) {
       acao_matrizTransicaoEstados[i][j] = NENHUMA_ACAO;
       proximo_estado_matrizTransicaoEstados[i][j] = i;
    }
  }
  proximo_estado_matrizTransicaoEstados[ESPERA][ACIONAR] = SAIDA;
  acao_matrizTransicaoEstados[ESPERA][ACIONAR] = A01;

  proximo_estado_matrizTransicaoEstados[SAIDA][DESACIONAR] = ESPERA;
  acao_matrizTransicaoEstados[SAIDA][DESACIONAR] = A03;

  proximo_estado_matrizTransicaoEstados[SAIDA][TIMEOUT] = ALERTA;
  acao_matrizTransicaoEstados[SAIDA][TIMEOUT] = A02;

  proximo_estado_matrizTransicaoEstados[ALERTA][DESACIONAR] = ESPERA;
  acao_matrizTransicaoEstados[ALERTA][DESACIONAR] = A04;

  proximo_estado_matrizTransicaoEstados[ALERTA][DISPARAR] = ENTRADA;
  acao_matrizTransicaoEstados[ALERTA][DISPARAR] = A05;

  proximo_estado_matrizTransicaoEstados[ENTRADA][TIMEOUT] = ACIONADO;
  acao_matrizTransicaoEstados[ENTRADA][TIMEOUT] = A06;

  proximo_estado_matrizTransicaoEstados[ENTRADA][DESACIONAR] = ESPERA;
  acao_matrizTransicaoEstados[ENTRADA][DESACIONAR] = A07;

  proximo_estado_matrizTransicaoEstados[ACIONADO][DESACIONAR] = ESPERA;
  acao_matrizTransicaoEstados[ACIONADO][DESACIONAR] = A07;


} // initStateMachine

/************************************************************************
 iniciaSistema
 Inicia o sistema ...
 Parametros de entrada: nenhum
 Retorno: nenhum
*************************************************************************/
void iniciaSistema()
{
   iniciaMaquinaEstados();
} // initSystem


/************************************************************************
 obterEvento
 Obtem um evento, que pode ser da IHM ou do alarme
 Parametros de entrada: nenhum
 Retorno: codigo do evento
*************************************************************************/
char* teclas;

int decodificarAcionar()
{
    if (teclas[2] == 'a')
    {
        if (sha.validar(teclas))
        {
            return true;
        }
    }
    return false;
}//decodificarAcionar

int decodificarDesacionar()
{
    if (teclas[2] == 'd')
    {
        if (sha.validar(teclas))
        {
            return true;
        }
    }
    return false;
}//decodificarDesacionar

int decodificarDisparar()
{
    if (teclas[0] == 'l')
    {
        return true;
    }
    return false;
}//decodificarDisparar

int decodificarTimeout()
{
    return tmr.timeout();
}

/************************************************************************
 obterAcao
 Obtem uma acao da Matriz de transicao de estados
 Parametros de entrada: estado (int)
                        evento (int)
 Retorno: codigo da acao
*************************************************************************/
int obterAcao(int estado, int codigoEvento) {
  return acao_matrizTransicaoEstados[estado][codigoEvento];
} // obterAcao


/************************************************************************
 obterProximoEstado
 Obtem o proximo estado da Matriz de transicao de estados
 Parametros de entrada: estado (int)
                        evento (int)
 Retorno: codigo do estado
*************************************************************************/
int obterProximoEstado(int estado, int codigoEvento) {
  return proximo_estado_matrizTransicaoEstados[estado][codigoEvento];
} // obterAcao



/***********************************************************************
 Tasks
 ***********************************************************************/

/************************************************************************
 taskMaqEstados
 Task principal de controle que executa a maquina de estados
 Parametros de entrada: nenhum
 Retorno: nenhum
*************************************************************************/
void taskMaqEstados(void *pvParameters) {
  int codigoEvento;
  BaseType_t xStatus;

  for( ;; ) {
    if( xQueueReceive( xQueue, &codigoEvento, portMAX_DELAY ) == pdPASS ) {
      if (codigoEvento != NENHUM_EVENTO) {
        codigoAcao = obterAcao(estado, codigoEvento);
        estado = obterProximoEstado(estado, codigoEvento);
        eventoInterno = executarAcao(codigoAcao);
        Serial.print("Estado: ");
        Serial.print(estado);
        Serial.print(" Evento: ");
        Serial.print(codigoEvento);
        Serial.print(" Acao: ");
        Serial.println(codigoAcao);

        // Descomente para imprimir ocupação do stack
        // Serial.print("Task stacks remaining: "); 
        // Serial.print(uxTaskGetStackHighWaterMark(xTaskMaqEstados)); Serial.print(" "); 
        // Serial.println(uxTaskGetStackHighWaterMark(xTaskObterEvento));
      }
    }
    else {
      Serial.println("Erro ao receber evento da fila");
    }
  }
}

/************************************************************************
 taskObterEvento
 Task que faz pooling de eventos
 Parametros de entrada: nenhum
 Retorno: nenhum
*************************************************************************/
void taskObterEvento(void *pvParameters) {
  int codigoEvento;
  BaseType_t xStatus;

  for( ;; ) {
    codigoEvento = NENHUM_EVENTO;

    teclas = ihm.obterTeclas();
    if (decodificarAcionar()) {
      codigoEvento = ACIONAR;
      xStatus = xQueueSendToBack( xQueue, &codigoEvento, 0 );
      if( xStatus != pdPASS )
        Serial.println("Erro ao enviar evento para fila");
      continue;
    }
    if (decodificarDesacionar()) {
      codigoEvento = DESACIONAR;
      xStatus = xQueueSendToBack( xQueue, &codigoEvento, 0 );
      if( xStatus != pdPASS )
        Serial.println("Erro ao enviar evento para fila");
      continue;
    }
    if (decodificarTimeout()) {
      codigoEvento = TIMEOUT;
      xStatus = xQueueSendToBack( xQueue, &codigoEvento, 0 );
      if( xStatus != pdPASS )
        Serial.println("Erro ao enviar evento para fila");
      continue;
    }
    if (decodificarDisparar()) {
      codigoEvento = DISPARAR;
      xStatus = xQueueSendToBack( xQueue, &codigoEvento, 0 );
      if( xStatus != pdPASS )
        Serial.println("Erro ao enviar evento para fila");
      continue;
    }
  }
}

/************************************************************************
 taskBipBip
 Task que bipa a sirene periodicamente caso acionada
 Parametros de entrada: nenhum
 Retorno: nenhum
*************************************************************************/
void taskBipBip(void *pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xDelay500ms = pdMS_TO_TICKS( 500 );

  for( ;; ) {
    xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );
    xLastWakeTime = xTaskGetTickCount();

    while(sne.estaAcionado()) {
      sne.bip();
      vTaskDelayUntil( &xLastWakeTime, xDelay500ms );
    }
  }
}

/************************************************************************
 Main
 Setup e Loop principal de controle que executa a maquina de estados
 Parametros de entrada: nenhum
 Retorno: nenhum
*************************************************************************/

void setup() {
  Serial.begin(9600);

  iniciaSistema();
  Serial.println("Alarme iniciado");

  // configure tasks
  xBinarySemaphore = xSemaphoreCreateBinary();
  xQueue = xQueueCreate(5, sizeof(int));
  if(xQueue != NULL && xBinarySemaphore != NULL)
  {
    xTaskCreate(taskMaqEstados,"taskMaqEstados", 150, NULL, 2, &xTaskMaqEstados);
    xTaskCreate(taskObterEvento,"taskObterEvento", 100, NULL, 1, &xTaskObterEvento);
    xTaskCreate(taskBipBip,"taskBipBip", 100, NULL, 1, &xTaskBipBip);
    vTaskStartScheduler();
  }
  else
  {
    /* The queue or semaphore could not be created. */
  }
} // setup

void loop() {
} // loop
