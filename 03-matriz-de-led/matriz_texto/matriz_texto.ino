#include <Adafruit_NeoPixel.h>

#define PINO 6
#define NLEDS 64
#define SERPENTINA 0   // 1 se as linhas impares sairem espelhadas

Adafruit_NeoPixel matriz(NLEDS, PINO, NEO_GRB + NEO_KHZ800);

// Fonte 5x7: cada byte e uma linha, bit 4 = coluna da esquerda
const uint8_t LETRA_G[7] = {0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01111};
const uint8_t LETRA_A[7] = {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001};
const uint8_t LETRA_Y[7] = {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100};

// Converte (x, y) da tela no indice do LED na fita
uint16_t xy(uint8_t x, uint8_t y) {
  if (SERPENTINA && (y & 1)) return y * 8 + (7 - x);
  return y * 8 + x;
}

void mostraLetra(const uint8_t letra[7], uint32_t cor) {
  matriz.clear();
  for (uint8_t y = 0; y < 7; y++) {
    for (uint8_t x = 0; x < 5; x++) {
      if (letra[y] & (1 << (4 - x))) {
        matriz.setPixelColor(xy(x + 1, y), cor);  // +1 centraliza 5 colunas em 8
      }
    }
  }
  matriz.show();
}

void setup() {
  matriz.begin();
  matriz.setBrightness(30);  // limita a corrente pra USB aguentar
}

void loop() {
  mostraLetra(LETRA_G, matriz.Color(255, 0, 0));    // vermelho
  delay(1000);
  mostraLetra(LETRA_A, matriz.Color(0, 255, 0));    // verde
  delay(1000);
  mostraLetra(LETRA_Y, matriz.Color(0, 0, 255));    // azul
  delay(1000);
  matriz.clear();
  matriz.show();
  delay(500);
}
