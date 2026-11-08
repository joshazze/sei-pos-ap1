int latchPin = 4;
int clockPin = 7;
int dataPin = 8;

byte numeros[10] = { //bit em 0 = segmento aceso
  B11000000, //0
  B11111001, //1
  B10100100, //2
  B10110000, //3
  B10011001, //4
  B10010010, //5
  B10000010, //6
  B11111000, //7
  B10000000, //8
  B10010000  //9
};
#define APAGADO B11111111
#define MENOS   B10111111

int cont = 0;
byte tela[4] = {APAGADO, APAGADO, APAGADO, B11000000}; //o que cada digito mostra
int dig = 0;     //digito aceso agora
int botao = 0;   //botao lido agora (0 = nenhum)
int parado = 0;  //quantos ticks de 2 ms o botao ficou igual

void manda(byte seg, byte dg) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, seg);
  shiftOut(dataPin, clockPin, MSBFIRST, dg); //1 2 4 8 = digito da esquerda pra direita
  digitalWrite(latchPin, HIGH);
}

void monta() { //cont -> tela, alinhado a direita
  int n = abs(cont);
  for (int i = 3; i >= 0; i--) {
    if (n > 0 || i == 3) {
      tela[i] = numeros[n % 10];
      n /= 10;
    } else if (cont < 0 && tela[i + 1] != MENOS && tela[i + 1] != APAGADO) {
      tela[i] = MENOS; //logo antes do primeiro algarismo
    } else {
      tela[i] = APAGADO;
    }
  }
}

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  for (int p = 10; p <= 13; p++) { //LEDs do shield apagados (acendem em LOW)
    pinMode(p, OUTPUT);
    digitalWrite(p, HIGH);
  }
  pinMode(3, OUTPUT); //buzzer calado (apita em LOW)
  digitalWrite(3, HIGH);

  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 124;            //16 MHz / 256 / 125 = 500 Hz: interrupcao a cada 2 ms
  TCCR1B |= (1 << WGM12); //zera no OCR1A
  TCCR1B |= (1 << CS12);  //prescaler 256
  TIMSK1 |= (1 << OCIE1A);
  interrupts();
}

void loop() {
}

ISR(TIMER1_COMPA_vect) {
  //display: um digito por vez, apagando antes pra nao vazar no vizinho
  manda(APAGADO, 0);
  manda(tela[dig], 1 << dig);
  dig = (dig + 1) % 4;

  //botoes (o shield tem pull-up: apertado = LOW)
  int agora = 0;
  if (digitalRead(A1) == LOW) agora = 1;
  else if (digitalRead(A2) == LOW) agora = 2;
  else if (digitalRead(A3) == LOW) agora = 3;

  if (agora != botao) { //mudou ou quicou: recomeca a contar
    botao = agora;
    parado = 0;
    return;
  }
  if (parado >= 25) return; //ja contou, espera soltar
  parado++;
  if (parado < 25) return;  //ainda nao firmou 50 ms

  if (botao == 1 && cont < 9999) cont++;
  if (botao == 2 && cont > -999) cont--;
  if (botao == 3) cont = 0;
  monta();
}
