#include "vfd_display.h"
#include "vfd_font.h"

// Pinos do display
static int _dataPin;
static int _clockPin;
static int _latchPin;
static int _blankPin;

// Buffer para os dados do display - 114 bits (0 a 113)
static byte displayData[15]; // 15 bytes = 120 bits, suficiente para os 114 bits

// Buffer para armazenar os caracteres atuais
static char textBuffer[2][VFD_MAX_COLUMNS + 1]; // [linha][coluna] +1 para o terminador nulo
static uint8_t currentColumn = 0;              // Coluna atual sendo renderizada
static unsigned long lastColumnChange = 0;      // Tempo da última alteração de coluna

// Variável para controle de tempo
static unsigned long lastColumnUpdate = 0;

// Função para definir um bit específico no buffer
static void setBit(int bitPos, bool value) {
  if (bitPos < 0 || bitPos > 113) return;
  
  int bytePos = bitPos / 8;
  int bitInByte = bitPos % 8;
  
  if (value) {
    displayData[bytePos] |= (1 << bitInByte);
  } else {
    displayData[bytePos] &= ~(1 << bitInByte);
  }
}

// Inicializa o display
void vfd_init(int dataPin, int clockPin, int latchPin, int blankPin) {
  // Salva os pinos
  _dataPin = dataPin;
  _clockPin = clockPin;
  _latchPin = latchPin;
  _blankPin = blankPin;
  
  // Configura os pinos
  pinMode(_latchPin, OUTPUT);
  pinMode(_blankPin, OUTPUT);
  
  // Inicializa SPI
  SPI.begin();
  // Configura SPI: MSB first, modo 0, velocidade máxima
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  
  // Estado inicial
  digitalWrite(_latchPin, LOW);
  digitalWrite(_blankPin, HIGH);
  
  // Limpa o display
  vfd_clear();
  
  // Liga o display
  digitalWrite(_blankPin, LOW);
  
  // Inicializa os buffers de texto com espaços
  for (int line = 0; line < 2; line++) {
    for (int i = 0; i < VFD_MAX_COLUMNS; i++) {
      textBuffer[line][i] = ' ';
    }
    textBuffer[line][VFD_MAX_COLUMNS] = '\0';
  }
  
  // Inicializa a variável de tempo
  lastColumnChange = millis();
}

// Limpa o display
void vfd_clear() {
  // Limpa o buffer de dados
  memset(displayData, 0, sizeof(displayData));
  
  // Limpa os buffers de texto
  for (int line = 0; line < 2; line++) {
    for (int i = 0; i < VFD_MAX_COLUMNS; i++) {
      textBuffer[line][i] = ' ';
    }
    textBuffer[line][VFD_MAX_COLUMNS] = '\0';
  }
  
  // Envia os dados limpos para o display
  vfd_update();
}

// Define texto para uma linha específica
void vfd_setTextLine(uint8_t line, const char* text) {
  if (line > 1) return; // Apenas 2 linhas disponíveis
  
  int len = strlen(text);
  // Limita o texto ao tamanho máximo
  if (len > VFD_MAX_COLUMNS) len = VFD_MAX_COLUMNS;
  
  // Copia o texto para o buffer
  for (int i = 0; i < len; i++) {
    textBuffer[line][i] = text[i];
  }
  
  // Preenche o restante com espaços
  for (int i = len; i < VFD_MAX_COLUMNS; i++) {
    textBuffer[line][i] = ' ';
  }
  
  textBuffer[line][VFD_MAX_COLUMNS] = '\0';
}

// Define texto para a primeira linha (compatibilidade com versão anterior)
void vfd_setText(const char* text) {
  vfd_setTextLine(0, text);
}

// Função auxiliar para renderizar um caractere em uma posição específica
void renderCharacter(char ch, uint8_t line, int colPosition) {
  // Converte para o índice ASCII (começa em espaço = 32)
  uint8_t charIndex = (uint8_t)ch - 32;
  
  // Validação do índice
  if (charIndex > 58) charIndex = 0; // Espaço para caracteres inválidos
  
  // Determina a posição inicial com base na linha
  int startBit = line == 0 ? VFD_LINE1_START : VFD_LINE2_START;
  
  // Para cada coluna da fonte do caractere
  for (int col = 0; col < 5; col++) {
    // Use a matriz de fonte correta: FONT_5X7
    byte columnPattern = FONT_5X7[charIndex][col];
    
    // Para cada bit do padrão
    for (int bit = 0; bit < 7; bit++) {
      if (columnPattern & (1 << bit)) {
        // Mapeamento correto para o formato de matriz 7x5
        int bitPos = startBit + (bit * 5) + col;
        
        // Certifica-se que o bit está dentro dos limites permitidos
        if (bitPos >= 0 && bitPos < 73) {
          setBit(bitPos, true);
        }
      }
    }
  }
}

// Renderiza o caractere atual da coluna em multiplexação
void vfd_renderCurrentColumn() {
  // Verifica se já passou tempo suficiente desde a última atualização
  unsigned long currentTime = micros();
  static unsigned long lastMicroUpdate = 0;
  
  if (currentTime - lastMicroUpdate < (COLUMN_CHANGE_INTERVAL * 1000)) {
    return;
  }
  lastMicroUpdate = currentTime;
  
  digitalWrite(_latchPin, LOW);
  
  // Limpa apenas os bits necessários no buffer ao invés de todo o buffer
  for (int i = 0; i < 15; i++) {
    displayData[i] = 0;
  }
  
  // Renderiza os caracteres de ambas as linhas simultaneamente
  char ch1 = textBuffer[0][currentColumn];
  char ch2 = textBuffer[1][currentColumn];
  renderCharacter(ch1, 0, currentColumn);
  renderCharacter(ch2, 1, currentColumn);
  
  // Ativa a coluna atual
  int controlBit1 = 73 + (currentColumn * 2);
  int controlBit2 = controlBit1 + 1;
  
  if (controlBit2 < VFD_TOTAL_BITS) {
    setBit(controlBit1, true);
    setBit(controlBit2, true);
  }
  
  // Envia os dados usando SPI
  noInterrupts(); // Desativa interrupções durante o envio crítico de dados
  for (int i = 14; i >= 0; i--) {
    SPI.transfer(displayData[i]);
  }
  interrupts(); // Reativa interrupções
  
  digitalWrite(_latchPin, HIGH);
  
  // Avança para a próxima coluna
  currentColumn = (currentColumn + 1) % VFD_MAX_COLUMNS;
}

// Função para atualizar o display - versão simplificada
void vfd_fastColumnUpdate() {
  vfd_renderCurrentColumn();
}

// Envia dados para o display - versão simplificada
void vfd_update() {
  digitalWrite(_latchPin, LOW);
  
  // Envia os dados usando SPI
  noInterrupts();
  for (int i = 14; i >= 0; i--) {
    SPI.transfer(displayData[i]);
  }
  interrupts();
  
  digitalWrite(_latchPin, HIGH);
} 

// Função de teste mostrando todos os caracteres
void vfd_test() {
  // Configura o texto com caracteres de teste em ambas as linhas
  vfd_setTextLine(0, "ABCDEFGHIJKLMNOPQRST");
  vfd_setTextLine(1, "01234567890123456789");
}
