#define TAM 4

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

void pd(const char *lt, int dg) {

  if (dg < 1 || dg >= (1 << TAM)) return; // 1 a 15

  int n = 0; //proxima letra de lt

  for (int i = 0; i < TAM; i++) {
    if (dg & (1 << (TAM - 1 - i))) { //o digito i tá marcado no dg?
      if (lt[n] == '\0') return; //para ao chegar no final do lt

      int p = lt[n] - 32; //a tabela comeca no espaco (32)
      if (p < 0 || p > 90) p = 0; //fora da tabela: apagado
      n++;

      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, ascii[p]);
      shiftOut(dataPin, clockPin, MSBFIRST, 1 << i); //1 2 4 8
      digitalWrite(latchPin, HIGH);
    }
  }
}

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
}

void loop() {

  unsigned long t;

  t = millis(); //valor atual do contador periférico
  while (millis() - t < 500) pd("A", 8);

  t = millis();
  while (millis() - t < 500) pd("SA", 12);

  t = millis();
  while (millis() - t < 500) pd("ASA", 14);

  t = millis();
  while (millis() - t < 500) pd("CASA", 15);

  t = millis();
  while (millis() - t < 500) pd("CAS", 7);

  t = millis();
  while (millis() - t < 500) pd("CA", 3);

  t = millis();
  while (millis() - t < 500) pd("C", 1);

  t = millis();
  while (millis() - t < 500) pd(" ", 8);

}
