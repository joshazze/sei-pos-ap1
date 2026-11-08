#define TAM 4

volatile bool rodar = false;
volatile int cont = 0;

const char *texto = " "; //o que o loop manda a pd mostrar agora
byte digitos = B1000; //nibble: 1 = digito aceso

int latchPin = 4;
int clockPin = 7;
int dataPin = 8;

int ascii[91] = { //do espaco (32) ao z (122): posicao = codigo - 32
  B11111111, //32  espaco
  0xFF,      //33  !  (nao existe)
  B11011101, //34  "
  0xFF,      //35  #  (nao existe)
  0xFF,      //36  $  (nao existe)
  0xFF,      //37  %  (nao existe)
  0xFF,      //38  &  (nao existe)
  B11011111, //39  '
  B11000110, //40  (
  B11110000, //41  )
  0xFF,      //42  *  (nao existe)
  0xFF,      //43  +  (nao existe)
  0xFF,      //44  ,  (nao existe)
  B10111111, //45  -
  B01111111, //46  .
  0xFF,      //47  /  (nao existe)

  B11000000, //48  0
  B11111001, //49  1
  B10100100, //50  2
  B10110000, //51  3
  B10011001, //52  4
  B10010010, //53  5
  B10000010, //54  6
  B11111000, //55  7
  B10000000, //56  8
  B10010000, //57  9

  0xFF,      //58  :  (nao existe)
  0xFF,      //59  ;  (nao existe)
  0xFF,      //60  <  (nao existe)
  B10110111, //61  =
  0xFF,      //62  >  (nao existe)
  B10101100, //63  ?
  0xFF,      //64  @  (nao existe)

  B10001000, //65  A
  B10000011, //66  B
  B11000110, //67  C
  B10100001, //68  D
  B10000110, //69  E
  B10001110, //70  F
  B11000010, //71  G
  B10001001, //72  H
  B11001111, //73  I
  B11100001, //74  J
  0xFF,      //75  K  (nao existe)
  B11000111, //76  L
  0xFF,      //77  M  (nao existe)
  B10101011, //78  N
  B11000000, //79  O
  B10001100, //80  P
  B10011000, //81  Q
  B10101111, //82  R
  B10010010, //83  S
  B10000111, //84  T
  B11000001, //85  U
  B11100011, //86  V
  0xFF,      //87  W  (nao existe)
  0xFF,      //88  X  (nao existe)
  B10010001, //89  Y
  B10100100, //90  Z

  B11000110, //91  [
  0xFF,      //92  barra invertida  (nao existe)
  B11110000, //93  ]
  0xFF,      //94  ^  (nao existe)
  B11110111, //95  _
  0xFF,      //96  `  (nao existe)

  B10100000, //97  a
  B10000011, //98  b
  B10100111, //99  c
  B10100001, //100  d
  B10000100, //101  e
  B10001110, //102  f
  B10010000, //103  g
  B10001011, //104  h
  B11111011, //105  i
  B11110001, //106  j
  0xFF,      //107  k  (nao existe)
  B11001111, //108  l
  0xFF,      //109  m  (nao existe)
  B10101011, //110  n
  B10100011, //111  o
  B10001100, //112  p
  B10011000, //113  q
  B10101111, //114  r
  B10010010, //115  s
  B10000111, //116  t
  B11100011, //117  u
  B11100011, //118  v
  0xFF,      //119  w  (nao existe)
  0xFF,      //120  x  (nao existe)
  B10010001, //121  y
  B10100100  //122  z
};

void shot(int dado, int clock, byte valor, int nbits) { //shiftOut com numero de bits escolhido
  for (int b = nbits - 1; b >= 0; b--) { //MSBFIRST: do bit mais alto pro mais baixo
    digitalWrite(dado, (valor >> b) & 1);
    digitalWrite(clock, HIGH); //o 595 le o dado na subida
    digitalWrite(clock, LOW);
  }
}

void pd(const char *lt, byte dg) { //dg em nibble: B0100 acende so o 2o digito

  if (dg < B0001 || dg > B1111) return; //nibble vazio ou fora dos 4 bits
  int n = 0; //proxima letra de lt

  for (int i = 0; i < TAM; i++) {
    if (dg & (1 << (TAM - 1 - i))) { //o digito i tá marcado no dg?
      if (lt[n] == '\0') return; //para ao chegar no final do lt

      int p = lt[n] - 32; //a tabela comeca no espaco (32)
      if (p < 0 || p > 90) p = 0; //fora da tabela: apagado
      n++;

      digitalWrite(latchPin, LOW);
      shot(dataPin, clockPin, ascii[p], 8);
      shot(dataPin, clockPin, 0, 4);      //Q4-Q7 do 595 dos digitos, sem uso: completa os 16 pulsos
      shot(dataPin, clockPin, 1 << i, 4); //nibble do digito: 1 2 4 8
      digitalWrite(latchPin, HIGH);
    }
  }
}

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  Serial.begin(9600);
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  OCR1A = 18749;

  TCCR1B |= (1<<WGM12);
  TCCR1B |= (1 << CS12);
  TIMSK1 |= (1 << OCIE1A);
  interrupts();
}

void loop() {

  if (rodar) {
    rodar = false;
    rotina300ms();
  }

  pd(texto, digitos); //varre o display o tempo todo com o quadro atual

}

ISR(TIMER1_COMPA_vect) {
  rodar = true;
}

void rotina300ms() {

  switch (cont) { //um quadro da animacao a cada 300 ms
    case 0: texto = "A";    digitos = B1000; break;
    case 1: texto = "SA";   digitos = B1100; break;
    case 2: texto = "ASA";  digitos = B1110; break;
    case 3: texto = "CASA"; digitos = B1111; break;
    case 4: texto = "CAS";  digitos = B0111; break;
    case 5: texto = "CA";   digitos = B0011; break;
    case 6: texto = "C";    digitos = B0001; break;
    case 7: texto = " ";    digitos = B1000; break;
  }

  cont++;
  if (cont > 7) cont = 0;

  Serial.println("rodou - 300ms" + cont);
}
