//FIGURAS NA MATRIZ WS2812B ESCOLHIDAS PELO TECLADO
//Alunos: Joshua e Bruna
//Serial Monitor em 9600: digitar 1, 2 ou 3 e dar Enter.
//  1 = raio amarelo
//  2 = triangulo vermelho
//  3 = coracao vermelho com PT em branco
//A figura fica acesa 1 s e a matriz apaga. Outra tecla no meio troca na hora
//e o 1 s recomeca.
//
//A MATRIZ
//Sao 64 LEDs em fila, cada um com um chip dentro. O DOUT de um vai no DIN do
//proximo, como os dois 74HC595 em cascata do shield. O Arduino manda 24 bits por
//LED; o primeiro chip guarda os 24 que chegam antes e repassa o resto.
//Muda o fio: e um so, sem clock e sem latch. Cada bit dura 1,25 us, e o tempo em
//HIGH decide o valor:
//  bit 0: 0,4 us em HIGH, 0,85 us em LOW
//  bit 1: 0,8 us em HIGH, 0,45 us em LOW
//O latch e o fio parado em LOW por mais de 50 us. Nessa hora os 64 trocam juntos.
//Ordem dos bits: G7..G0, R7..R0, B7..B0, o mais alto primeiro (MSBFIRST).
//Ligacao: DIN no pino 6, 5V no 5V, GND no GND. O chip le HIGH acima de 3,5 V
//(0,7 x 5 V), entao o pino do Uno liga direto.

#include <Adafruit_NeoPixel.h> //externa, instalada pelo gerenciador de bibliotecas

//A BIBLIOTECA ADAFRUIT NEOPIXEL
//Guarda a imagem num array na RAM, 3 bytes por LED: 192 bytes pros 64.
//setPixelColor, clear e setBrightness mexem so nesse array. O fio fica parado.
//O show e que transmite. A 16 MHz um ciclo de clock dura 62,5 ns, e 1 bit de
//1,25 us cabe em 20 ciclos. Um trecho em assembly conta esses ciclos bit a bit
//(liga o pino, espera, desliga) pros 1536 bits da matriz, uns 1,9 ms.
//Nesse tempo as interrupcoes ficam desligadas: uma ISR no meio esticaria um bit
//e o chip leria errado. A do Timer1 espera e roda logo depois do show.
//
//  Adafruit_NeoPixel matriz(n, pino, tipo)  cria o objeto e reserva o array
//  matriz.begin()                           pinMode(pino, OUTPUT)
//  matriz.setBrightness(b)                  escala todas as cores por b/255
//  matriz.Color(r, g, b)                    junta 3 bytes num numero de 32 bits
//  matriz.setPixelColor(n, cor)             grava a cor do LED n no array
//  matriz.clear()                           zera o array
//  matriz.show()                            transmite o array e faz o reset

#define PINO 6
#define NLEDS 64
#define SERPENTINA 0 //1 se as linhas impares sairem espelhadas (ver xy)
#define TEMPO 100    //1 s = 100 interrupcoes de 10 ms

//NEO_GRB: o chip espera verde, vermelho, azul. NEO_KHZ800: 800 mil bits por segundo
Adafruit_NeoPixel matriz(NLEDS, PINO, NEO_GRB + NEO_KHZ800);

//variaveis que a ISR muda e o loop le: volatile, senao o compilador guarda
//uma copia num registrador e o loop nunca ve a troca
volatile int figura = 0;       //0 = apagada, 1 a 3 = figura na tela
volatile int resta = 0;        //quantos ticks de 10 ms faltam pra apagar
volatile bool mudou = false;   //avisa o loop que precisa redesenhar

//FIGURAS
//Um byte por linha, de cima pra baixo. Bit 7 (o da esquerda no B...) = coluna 0.
//1 acende, 0 apaga. E a tabela de segmentos do display com a logica invertida:
//la, no anodo comum, quem acendia era o 0.
byte raio[8] = {
  B00011110,
  B00111100,
  B01111000,
  B11111110,
  B00011100,
  B00111000,
  B00110000,
  B00100000
};

byte triangulo[8] = {
  B00011000,
  B00011000,
  B00111100,
  B00111100,
  B01111110,
  B01111110,
  B11111111,
  B11111111
};

byte coracao[8] = {
  B01100110,
  B11111111,
  B11111111,
  B11111111,
  B01111110,
  B00111100,
  B00011000,
  B00000000
};

byte pt[8] = { //P nas colunas 1 a 3, T nas colunas 5 a 7, linhas 1 a 4
  0,
  B01110111,
  B01010010,
  B01110010,
  B01000010,
  0, 0, 0
};

//A biblioteca numera os LEDs de 0 a 63 na ordem da fila. A linha y comeca no
//LED y * 8, entao (x, y) vira y * 8 + x. Em zigue-zague a linha impar corre ao
//contrario e a coluna vira 7 menos x.
int xy(int x, int y) {
  if (SERPENTINA && y % 2 == 1) return y * 8 + (7 - x);
  return y * 8 + x;
}

//B10000000 >> x leva o 1 pra coluna x. O & com a linha da 0 se o bit esta apagado.
void desenha(byte img[8], unsigned long cor) {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (img[y] & (B10000000 >> x)) matriz.setPixelColor(xy(x, y), cor);
    }
  }
}

void mostra(int f) {
  matriz.clear();
  switch (f) {
    case 1:
      desenha(raio, matriz.Color(255, 200, 0)); //amarelo
      break;
    case 2:
      desenha(triangulo, matriz.Color(255, 0, 0)); //vermelho
      break;
    case 3:
      desenha(coracao, matriz.Color(255, 0, 0)); //vermelho
      desenha(pt, matriz.Color(255, 255, 255));  //branco por cima do coracao
      break;
  }
  matriz.show(); //f = 0 cai aqui com o array zerado: apaga
}

//TECLADO POR INTERRUPCAO
//O byte digitado chega pela USART. A propria Serial ja tem uma ISR de recepcao
//que joga cada byte num buffer de 64. Por isso nao da pra escrever outra
//ISR(USART_RX_vect): ela ja existe e o compilador acusa duplicada.
//A do Timer1, a cada 10 ms, tira do buffer o que chegou e conta o 1 s da figura.
//O loop so redesenha quando ela avisa.
ISR(TIMER1_COMPA_vect) {
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c >= '1' && c <= '3') {
      figura = c - '0'; //'1' e 49 na ASCII e '0' e 48: subtraindo, sobra o numero 1
      resta = TEMPO;
      mudou = true;
    }
  }

  if (resta > 0) {
    resta--;
    if (resta == 0) { //passou 1 s: apaga
      figura = 0;
      mudou = true;
    }
  }
}

void setup() {
  Serial.begin(9600);
  matriz.begin();
  matriz.setBrightness(30); //64 LEDs em branco cheio passam de 3 A (estimativa de
                            //60 mA por LED); a USB da 500 mA. Em 30 fica em uns 450 mA
  mostra(0);

  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 624;            //16 MHz / 256 / 625 = 100 Hz: interrupcao a cada 10 ms
  TCCR1B |= (1 << WGM12); //zera no OCR1A
  TCCR1B |= (1 << CS12);  //prescaler 256
  TIMSK1 |= (1 << OCIE1A);
  interrupts();

  Serial.println("Digite 1, 2 ou 3");
}

void loop() {
  if (mudou) {
    noInterrupts(); //copia as duas variaveis sem a ISR mexer no meio
    int f = figura;
    mudou = false;
    interrupts();

    mostra(f); //show fica fora da ISR: leva 1,9 ms com interrupcao desligada
    if (f == 0) Serial.println("apagou");
    else {
      Serial.print("figura ");
      Serial.println(f);
    }
  }
}
