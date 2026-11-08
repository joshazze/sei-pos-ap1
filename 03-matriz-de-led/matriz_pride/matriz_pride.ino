#include <Adafruit_NeoPixel.h>

#define PINO 6
#define NLEDS 64
#define SERPENTINA 0 //1 se as linhas impares sairem espelhadas
#define NBANDEIRAS 5
#define NTEXTO 33

Adafruit_NeoPixel matriz(NLEDS, PINO, NEO_GRB + NEO_KHZ800);

byte arco[8][3] = { //8 cores do arco-iris, em R G B
  {255, 0, 0},   //vermelho
  {255, 100, 0}, //laranja
  {255, 220, 0}, //amarelo
  {0, 200, 0},   //verde
  {0, 200, 200}, //ciano
  {0, 60, 255},  //azul
  {140, 0, 200}, //roxo
  {255, 30, 140} //rosa
};

byte bandeiras[NBANDEIRAS][8][3] = { //uma cor por linha da matriz, de cima pra baixo
  { //arco-iris
    {255, 0, 0}, {255, 0, 0}, {255, 100, 0}, {255, 220, 0},
    {0, 200, 0}, {0, 60, 255}, {140, 0, 200}, {140, 0, 200}
  },
  { //trans
    {40, 170, 255}, {40, 170, 255}, {255, 90, 140}, {255, 255, 255},
    {255, 255, 255}, {255, 90, 140}, {40, 170, 255}, {40, 170, 255}
  },
  { //bi
    {230, 0, 110}, {230, 0, 110}, {230, 0, 110}, {120, 40, 160},
    {120, 40, 160}, {0, 40, 200}, {0, 40, 200}, {0, 40, 200}
  },
  { //lesbica
    {220, 40, 0}, {220, 40, 0}, {255, 130, 50}, {255, 255, 255},
    {255, 255, 255}, {220, 90, 170}, {170, 0, 90}, {170, 0, 90}
  },
  { //pan
    {255, 30, 140}, {255, 30, 140}, {255, 30, 140}, {255, 210, 0},
    {255, 210, 0}, {0, 180, 255}, {0, 180, 255}, {0, 180, 255}
  }
};

byte texto[NTEXTO] = { //GAY em colunas: bit 7 = linha de cima
  0, 0, 0, 0, 0, 0, 0, 0, //entra pela direita
  B01111100, B10000010, B10010010, B10010010, B01011110, 0, //G
  B01111110, B10010000, B10010000, B10010000, B01111110, 0, //A
  B11000000, B00100000, B00011110, B00100000, B11000000,    //Y
  0, 0, 0, 0, 0, 0, 0, 0  //sai pela esquerda
};

byte coracao[8] = { //uma linha por byte: bit 7 = coluna da esquerda
  B01100110,
  B11111111,
  B11111111,
  B11111111,
  B01111110,
  B00111100,
  B00011000,
  B00000000
};

byte onda[8] = {255, 200, 130, 70, 40, 70, 130, 200}; //brilho que corre pelas colunas
byte batida[10] = {255, 120, 230, 80, 80, 80, 80, 80, 80, 80}; //tum-tum, 100 ms por passo

int xy(int x, int y) { //coluna e linha -> numero do LED na fita
  if (SERPENTINA && y % 2 == 1) return y * 8 + (7 - x);
  return y * 8 + x;
}

void pinta(int x, int y, byte cor[3], byte brilho) { //acende um LED com a cor escurecida
  matriz.setPixelColor(xy(x, y), matriz.Color(
    map(brilho, 0, 255, 0, cor[0]),
    map(brilho, 0, 255, 0, cor[1]),
    map(brilho, 0, 255, 0, cor[2])));
}

void rolaTexto() { //janela de 8 colunas andando pelo texto
  for (int d = 0; d <= NTEXTO - 8; d++) {
    matriz.clear();
    for (int x = 0; x < 8; x++) {
      for (int y = 0; y < 8; y++) {
        if (texto[d + x] & (B10000000 >> y)) pinta(x, y, arco[(d + x) % 8], 255);
      }
    }
    matriz.show();
    delay(90);
  }
}

void bandeira(int b) { //3 s com a onda de brilho andando
  for (int passo = 0; passo < 60; passo++) {
    for (int x = 0; x < 8; x++) {
      for (int y = 0; y < 8; y++) {
        pinta(x, y, bandeiras[b][y], onda[(x + passo) % 8]);
      }
    }
    matriz.show();
    delay(50);
  }
}

void bateCoracao() { //5 batidas, arco-iris descendo
  for (int passo = 0; passo < 50; passo++) {
    matriz.clear();
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (coracao[y] & (B10000000 >> x)) pinta(x, y, arco[(y + passo) % 8], batida[passo % 10]);
      }
    }
    matriz.show();
    delay(100);
  }
}

void arcoDiagonal() { //cada diagonal numa cor, girando
  for (int passo = 0; passo < 80; passo++) {
    for (int x = 0; x < 8; x++) {
      for (int y = 0; y < 8; y++) {
        pinta(x, y, arco[(x + y + passo) % 8], 255);
      }
    }
    matriz.show();
    delay(75);
  }
}

void setup() {
  matriz.begin();
  matriz.setBrightness(30); //limita a corrente pra USB aguentar
}

void loop() {
  rolaTexto();
  for (int b = 0; b < NBANDEIRAS; b++) bandeira(b);
  bateCoracao();
  arcoDiagonal();
}
