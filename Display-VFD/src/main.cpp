#include <Arduino.h>
#include "vfd_display.h"

// Definição dos pinos
const int dataPin = 2;    // Pino de dados (SI)
const int clockPin = 3;   // Pino de clock (CLK)
const int latchPin = 4;   // Pino de latch (LAT)
const int blankPin = 5;   // Pino de blank (BK)
const int ledPin = 13;    // Pino do LED

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
  // Atualização simples do display - uma coluna por vez
  vfd_renderCurrentColumn();
  
  // Atualiza o estado do pisca do LED se necessário
  if (isBlinking) {
    updateLedBlink();
  }
  
  // Verifica se é hora de mudar o modo de demonstração
  unsigned long currentMillis = millis();
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
    
    // Inicia o pisca do LED para indicar mudança de modo
    startLedBlink();
  }
  
  // Processa comandos da porta serial
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

