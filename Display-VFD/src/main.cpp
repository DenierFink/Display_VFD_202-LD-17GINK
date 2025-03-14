#include <Arduino.h>
#include <SPI.h>
#include "vfd_display.h"

// Definição dos pinos - usando hardware SPI
const int dataPin = MOSI;    // Pino 51 no Mega
const int clockPin = SCK;    // Pino 52 no Mega
const int latchPin = SS;     // Pino 53 no Mega
const int blankPin = 5;      // Mantém o mesmo pino blank
const int ledPin = 13;       // Pino do LED

// Variáveis para controle de tempo
unsigned long lastDebugTime = 0;

// Modo de demonstração
enum DemoMode {
  ALPHABET_DEMO,
  NUMBERS_DEMO,
  TEXT_DEMO
};

DemoMode currentMode = ALPHABET_DEMO;
unsigned long lastModeChangeTime = 0;
const unsigned long modeChangeDuration = 5000; // 5 segundos por modo

// Variáveis para implementar pisca sem delay
unsigned long blinkStartTime = 0;
bool isBlinking = false;
int blinkCount = 0;
const int BLINK_INTERVAL = 100;  // 100ms para cada estado do LED (on/off)
const int BLINK_TIMES = 3;      // Número de vezes para piscar

// Função para iniciar o pisca do LED
void startLedBlink() {
  isBlinking = true;
  blinkCount = 0;
  blinkStartTime = millis();
  digitalWrite(ledPin, HIGH);
}

// Função para atualizar o pisca do LED sem usar delay
void updateLedBlink() {
  if (!isBlinking) return;
  
  unsigned long currentTime = millis();
  
  if (currentTime - blinkStartTime >= BLINK_INTERVAL) {
    blinkStartTime = currentTime;
    
    // Determina o estado do LED com base na paridade do blinkCount
    bool shouldBeOn = (blinkCount % 2) == 0;
    digitalWrite(ledPin, shouldBeOn ? HIGH : LOW);
    
    blinkCount++;
    if (blinkCount >= BLINK_TIMES * 2) {
      isBlinking = false;
      digitalWrite(ledPin, LOW); // Garante que o LED termina desligado
    }
  }
}

void setup() {
  // Configura o LED indicador
  pinMode(ledPin, OUTPUT);
  
  // Inicializa a comunicação serial
  Serial.begin(9600);
  Serial.println("Inicializando Display VFD...");
  
  // Inicializa o display
  vfd_init(dataPin, clockPin, latchPin, blankPin);
  
  // Define texto inicial para ambas as linhas
  vfd_setTextLine(0, "DISPLAY VFD TESTE");
  vfd_setTextLine(1, "LINHA 2 ATIVADA");
  
  Serial.println("Inicializacao concluida!");
}

void loop() {
  // Atualiza o display o mais rápido possível
  vfd_renderCurrentColumn();
  
  // Processamento de eventos em intervalos específicos
  unsigned long currentMillis = millis();
  
  // Atualiza o estado do pisca do LED se necessário
  if (isBlinking) {
    updateLedBlink();
  }
  
  // Verifica mudança de modo em intervalos
  static unsigned long lastModeCheck = 0;
  if (currentMillis - lastModeCheck >= 100) { // Verifica a cada 100ms
    lastModeCheck = currentMillis;
    
    if (currentMillis - lastModeChangeTime >= modeChangeDuration) {
      lastModeChangeTime = currentMillis;
      
      // Alterna entre os modos de demonstração
      switch (currentMode) {
        case ALPHABET_DEMO:
          vfd_setTextLine(0, "ABCDEFGHIJKLMNOPQRST");
          vfd_setTextLine(1, "TESTE LINHA 2 ABCDE");
          currentMode = NUMBERS_DEMO;
          break;
          
        case NUMBERS_DEMO:
          vfd_setTextLine(0, "01234567890123456789");
          vfd_setTextLine(1, "98765432109876543210");
          currentMode = TEXT_DEMO;
          break;
          
        case TEXT_DEMO:
          vfd_setTextLine(0, "DISPLAY VFD TESTE");
          vfd_setTextLine(1, "FUNCIONANDO BEM!");
          currentMode = ALPHABET_DEMO;
          break;
      }
      
      startLedBlink();
    }
  }
  
  // Verifica comandos seriais em intervalos
  static unsigned long lastSerialCheck = 0;
  if (currentMillis - lastSerialCheck >= 50) { // Verifica a cada 50ms
    lastSerialCheck = currentMillis;
    
    if (Serial.available() > 0) {
      String command = Serial.readStringUntil('\n');
      if (command.startsWith("L1:")) {
        vfd_setTextLine(0, command.substring(3).c_str());
        Serial.println("Linha 1 atualizada");
      }
      else if (command.startsWith("L2:")) {
        vfd_setTextLine(1, command.substring(3).c_str());
        Serial.println("Linha 2 atualizada");
      }
      else if (command == "TEST") {
        vfd_test();
        Serial.println("Modo de teste ativado");
      }
      else if (command == "CLEAR") {
        vfd_clear();
        Serial.println("Display limpo");
      }
    }
  }
}

