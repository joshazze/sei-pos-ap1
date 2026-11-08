//IMPRIME NA MATRIZ WS2812B O CARACTERE DIGITADO NO TECLADO
//Alunos: Joshua e Bruna
//Caracteres implementados: G, A e Y (maiusculo ou minusculo).
//Uso: abrir o Serial Monitor em 9600, digitar a letra e dar Enter.
//
//MATRIZ 8x8 DE WS2812B
//Cada um dos 64 LEDs tem um chip dentro e eles estao ligados em fila: o DOUT de um
//vai no DIN do proximo. Mesma ideia dos dois 74HC595 em cascata do shield: o Arduino
//manda 24 bits por LED, cada chip guarda os primeiros 24 que recebe e repassa o resto.
//Diferenca: e um fio so, sem clock e sem latch. Cada bit dura 1,25 us e o que diz se
//ele e 0 ou 1 e o tempo em HIGH (0,4 us = bit 0, 0,8 us = bit 1). O latch e o fio
//parado em LOW por mais de 50 us: ai todos os LEDs mostram o que guardaram.
//Ordem dos 24 bits: G7..G0, R7..R0, B7..B0, o mais alto primeiro.
//Microssegundo e rapido demais pro digitalWrite, por isso quem gera o sinal e a
//biblioteca Adafruit NeoPixel.
//Ligacao: DIN no pino 6, 5V no 5V, GND no GND.

#include <Adafruit_NeoPixel.h> //biblioteca externa, instalada pelo gerenciador de bibliotecas

//O QUE A BIBLIOTECA ADAFRUIT NEOPIXEL FAZ
//1. Guarda a imagem na RAM: um array de 3 bytes por LED (64 x 3 = 192 bytes).
//   setPixelColor, clear e setBrightness so mexem nesse array, nao tocam no fio.
//2. No show, transforma o array no sinal do WS2812B. O Uno roda a 16 MHz, entao
//   cada ciclo de clock dura 62,5 ns e um bit de 1,25 us sao 20 ciclos. A biblioteca
//   tem um trecho em assembly que conta esses ciclos: liga o pino, espera 6 ou 13
//   ciclos conforme o bit seja 0 ou 1, desliga, completa os 20, e repete pros
//   64 x 24 = 1536 bits (uns 1,9 ms).
//3. Desliga as interrupcoes durante o show (noInterrupts), porque uma interrupcao
//   no meio esticaria um bit e o chip leria errado. Efeito colateral: nesses 1,9 ms
//   o millis nao conta e a Serial pode perder um caractere que chegue bem na hora.
//4. Garante o reset entre um show e o proximo: espera o fio ficar em LOW o tempo
//   minimo antes de mandar de novo (o chip pede mais de 50 us).
//
//Funcoes usadas neste codigo:
//  Adafruit_NeoPixel matriz(n, pino, tipo)  cria o objeto e reserva o array
//  matriz.begin()                           pinMode(pino, OUTPUT)
//  matriz.setBrightness(b)                  escala todas as cores por b/255
//  matriz.Color(r, g, b)                    junta 3 bytes num numero de 32 bits
//  matriz.setPixelColor(n, cor)             grava a cor do LED n no array
//  matriz.clear()                           zera o array inteiro
//  matriz.show()                            manda o array pro fio (passo 2 a 4)

#define PINO 6
#define NLEDS 64
#define SERPENTINA 0 //1 se as linhas impares sairem espelhadas (ver xy)

//cria a matriz: 64 LEDs no pino 6
//NEO_GRB: o chip espera a cor na ordem verde, vermelho, azul
//NEO_KHZ800: sinal de 800 kHz, o do WS2812B
Adafruit_NeoPixel matriz(NLEDS, PINO, NEO_GRB + NEO_KHZ800);

//CARACTERES
//Um byte por linha, de cima pra baixo. Cada bit e um LED da linha:
//bit 7 (o da esquerda no B...) = coluna 0. 1 = aceso, 0 = apagado.
//Mesma logica da tabela de segmentos do display, so que aqui o 1 acende.
byte letraG[8] = {
  B00111000,
  B01000100,
  B01000000,
  B01011100,
  B01000100,
  B01000100,
  B00111100,
  B00000000
};

byte letraA[8] = {
  B00111000,
  B01000100,
  B01000100,
  B01111100,
  B01000100,
  B01000100,
  B01000100,
  B00000000
};

byte letraY[8] = {
  B01000100,
  B01000100,
  B00101000,
  B00010000,
  B00010000,
  B00010000,
  B00010000,
  B00000000
};

//A biblioteca so conhece o LED 0, 1, 2, ..., 63, na ordem da fila.
//Esta funcao converte coluna x e linha y nesse numero: a linha y comeca no
//LED y * 8, entao o LED e y * 8 + x. Em zigue-zague as linhas impares vao da
//direita pra esquerda e a coluna fica invertida (7 menos x).
int xy(int x, int y) {
  if (SERPENTINA && y % 2 == 1) return y * 8 + (7 - x);
  return y * 8 + x;
}

//Percorre as 8 linhas e as 8 colunas do caractere.
//B10000000 >> x anda com o 1 pra direita: x = 0 testa o bit 7, x = 7 testa o bit 0.
//setPixelColor so grava a cor num array na RAM, show e que manda pros LEDs.
void imprime(byte letra[8], unsigned long cor) {
  matriz.clear(); //zera o array: apaga o caractere anterior
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (letra[y] & (B10000000 >> x)) matriz.setPixelColor(xy(x, y), cor);
    }
  }
  matriz.show(); //manda os 64 x 24 bits e o reset: os LEDs mudam juntos
}

void setup() {
  Serial.begin(9600);
  matriz.begin();           //configura o pino 6 como saida
  matriz.setBrightness(30); //brilho de 0 a 255: 64 LEDs em branco cheio passam de 3 A,
                            //mais do que a USB aguenta (500 mA)
  matriz.clear();
  matriz.show();
  Serial.println("Digite G, A ou Y");
}

void loop() {
  if (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') return; //Enter do Serial Monitor, ignora

    switch (c) {
      case 'G':
      case 'g':
        imprime(letraG, matriz.Color(255, 0, 0)); //vermelho
        Serial.println("G");
        break;

      case 'A':
      case 'a':
        imprime(letraA, matriz.Color(0, 255, 0)); //verde
        Serial.println("A");
        break;

      case 'Y':
      case 'y':
        imprime(letraY, matriz.Color(0, 0, 255)); //azul
        Serial.println("Y");
        break;

      default: //caractere nao implementado: apaga a matriz
        matriz.clear();
        matriz.show();
        Serial.print(c);
        Serial.println(" nao implementado, use G, A ou Y");
    }
  }
}
