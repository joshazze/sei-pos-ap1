// Executa uma rotina a cada 300 ms usando Timer1 e interrupção
// Arduino UNO / Nano - ATmega328P
// Clock = 16 MHz

volatile bool executaRotina = false;  //nao precisa ser otimizada pelo compilador, mudada a qualquer momento por qualquer um

void setup() {
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);

  // Desabilita interrupções durante a configuração
  noInterrupts();

  // Limpa os registradores do Timer1
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  /*
     Timer1:
     Clock Arduino = 16 MHz

     Prescaler = 256

     Frequência do timer:
     16.000.000 / 256 = 62.500 Hz

     Cada contagem:
     1 / 62.500 = 16 us

     Para 300 ms:

     300 ms / 16 us = 18.750 contagens

     Como começa em zero:
     OCR1A = 18.749
  */

  OCR1A = 18749;

  // Modo CTC
  // Clear Timer on Compare Match
  TCCR1B |= (1 << WGM12); //constantes simbolicas

  // Prescaler = 256
  TCCR1B |= (1 << CS12);

  // Habilita interrupção por comparação A
  TIMSK1 |= (1 << OCIE1A);

  // Reabilita interrupções
  interrupts();
}

void loop() {

  if (executaRotina) {

    // Limpa a flag
    executaRotina = false;

    // Rotina executada a cada 300 ms
    rotina300ms();
  }

  // O restante do programa pode executar normalmente aqui
}


// ----------------------------------------------------
// Interrupção do Timer1
// ----------------------------------------------------
ISR(TIMER1_COMPA_vect) { //funcao padrao de 300ms, padrãozao

  // Não execute tarefas demoradas aqui.
  // Apenas sinaliza para o loop()
  executaRotina = true;
}


// ----------------------------------------------------
// Rotina executada a cada 300 ms
// ----------------------------------------------------
void rotina300ms() {

  static bool estadoLED = false;

  estadoLED = !estadoLED;

  digitalWrite(LED_BUILTIN, estadoLED);

  Serial.println("Passaram 300 ms");
}
