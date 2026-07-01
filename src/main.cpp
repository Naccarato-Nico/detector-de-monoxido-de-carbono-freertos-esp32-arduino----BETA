#include <Arduino.h>

const int PIN_MQ7    = 34;  
const int LED_VERDE  = 25;  
const int LED_ROJO   = 26;  
const int LED_AZUL   = 27;  
const int PIN_BUZZER = 14;  
const int PIN_BOTON  = 12;  

const int UMBRAL_ALERTA = 1600; 

volatile int lecturaGasGlobal = 0;   
volatile bool modoTestActivo  = false;  

TaskHandle_t TareaSensorHandle = NULL;
TaskHandle_t TareaAlarmaHandle = NULL;
TaskHandle_t TareaBotonHandle  = NULL;

void vTareaSensor(void *pvParameters) {
  for(;;) { 
    lecturaGasGlobal = analogRead(PIN_MQ7); 
    Serial.print("[Core 1] Nivel de CO: ");
    Serial.println(lecturaGasGlobal);

    vTaskDelay(pdMS_TO_TICKS(500)); 
  }
}

void vTareaBoton(void *pvParameters) {
  for(;;) {
    if (digitalRead(PIN_BOTON) == LOW && !modoTestActivo) {
      modoTestActivo = true; 
      vTaskDelay(pdMS_TO_TICKS(1000)); 
      modoTestActivo = false; 
    }
    vTaskDelay(pdMS_TO_TICKS(50)); 
  }
}

void vTareaAlarma(void *pvParameters) {
  for(;;) {
    if (modoTestActivo) {
      digitalWrite(LED_VERDE, LOW);
      digitalWrite(LED_ROJO, LOW);
      digitalWrite(LED_AZUL, HIGH);
      ledcWriteTone(0, 800);
      vTaskDelay(pdMS_TO_TICKS(100)); 
    }
    else if (lecturaGasGlobal >= UMBRAL_ALERTA) {
      digitalWrite(LED_VERDE, LOW);
      digitalWrite(LED_AZUL, LOW);
      
      digitalWrite(LED_ROJO, HIGH);
      ledcWriteTone(0, 1500);
      vTaskDelay(pdMS_TO_TICKS(200)); 
      
      digitalWrite(LED_ROJO, LOW);
      ledcWriteTone(0, 0);
      vTaskDelay(pdMS_TO_TICKS(200)); 
    } 
    else {
      digitalWrite(LED_VERDE, HIGH);
      digitalWrite(LED_ROJO, LOW);
      digitalWrite(LED_AZUL, LOW);
      ledcWriteTone(0, 0);
      vTaskDelay(pdMS_TO_TICKS(100)); 
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_MQ7, INPUT);
  pinMode(PIN_BOTON, INPUT_PULLUP); 
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);

  ledcSetup(0, 1000, 8);
  ledcAttachPin(PIN_BUZZER, 0);

  xTaskCreatePinnedToCore(vTareaSensor, "LeerSensor", 2048, NULL, 2, &TareaSensorHandle, 1); 
  xTaskCreatePinnedToCore(vTareaBoton,  "LeerBoton",  2048, NULL, 2, &TareaBotonHandle,  1); 
  xTaskCreatePinnedToCore(vTareaAlarma, "CtrlAlarma", 2048, NULL, 1, &TareaAlarmaHandle, 0); 

  Serial.println("--- Sistema Multitarea FreeRTOS Iniciado ---");
}

void loop() {
  vTaskDelete(NULL); 
}