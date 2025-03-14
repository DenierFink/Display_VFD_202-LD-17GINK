#ifndef VFD_DISPLAY_H
#define VFD_DISPLAY_H

#include <Arduino.h>
#include <SPI.h>

// Constantes do display
#define VFD_MAX_COLUMNS 20    // Máximo de colunas suportadas pelo display
#define VFD_TOTAL_BITS 114    // Total de bits a serem enviados ao display
#define VFD_LINE1_START 0     // Primeiro bit da linha 1
#define VFD_LINE2_START 36    // Primeiro bit da linha 2
#define COLUMN_CHANGE_INTERVAL 1 // Reduzido para 1ms já que SPI é mais rápido

// Funções para controle do display
void vfd_init(int dataPin, int clockPin, int latchPin, int blankPin);
void vfd_setChar(uint8_t column, char character);
void vfd_setText(const char* text);
void vfd_setTextLine(uint8_t line, const char* text); // Define texto para uma linha específica
void vfd_update();
void vfd_clear();
void vfd_test(); // Função de teste exibindo padrões
void vfd_renderCurrentColumn(); // Função para renderizar a coluna atual em multiplexação
void vfd_fastColumnUpdate(); // Nova função para atualização rápida de colunas
void renderCharacter(char ch, uint8_t line, int colPosition); // Função auxiliar para renderizar caracteres

#endif // VFD_DISPLAY_H
