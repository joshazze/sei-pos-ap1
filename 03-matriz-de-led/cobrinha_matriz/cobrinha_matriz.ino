// ----===---===---- SETUP INICIAL ----===---===----

//pinos da matriz

#define PINO_IN 6 //IN da matriz (no Uno: bit 6 do PORTD)

#define PINO_HIGH PORTD |= B01000000  //pino 6 em HIGH (|= so mexe no bit 6: os pinos 0 e 1 do PORTD sao o RX/TX do serial; vira sbi, 2 ciclos = 125 ns, igual o PORTH do Mega)
#define PINO_LOW  PORTD &= ~B01000000 //pino 6 em LOW (vira cbi, tambem 125 ns)

//globais da cobra
static int c_perm[64][2]; //64 (tijolinhos da cobra) * 2 (X Y)
static int c_tam = 1; //tamanho atual da cobra (tijolinhos validos vao de c_perm[0] ate c_perm[c_tam - 1])

//globais da fruta
static int f_perm [10][2]; //10 para guardar o histórico dos ultimos 5 lugares e não repetir, 2 para o x e o y

//globais do jogo
static volatile char g_dir = 'd'; //volatile: a interrupção muda ela por fora do loop, entao o compilador tem que ler da RAM toda vez | global direção: 'w' cima, 's' baixo, 'a' esquerda, 'd' direita (começa pra direita)

//globais da matriz de led
static byte m_perm [64][3]; //64 (fila de leds) * 3 (R G B)

// ----===---===---- FUNÇÕES DE LED DA MATRIZ ----===---===----

void mb(byte v) {
//manda o bit do 7 pro 0 (datasheet: high bit sent at first) do numero inteiro recebido
  for (byte i = 0; i < 8; i++) { 
    PINO_HIGH; //sobe
    PINO_HIGH; //repete: o fio nao muda, so segura mais 125 ns
    PINO_HIGH;
    if ((v & 0x80) == 0) { //olhamos o primeiro bit da esquerda pra direita do "v", se for zero:
      PINO_LOW;  //manda 0
    } else { //se não for zero:
      PINO_HIGH; //manda 1
    }
    PINO_LOW;    //mandamos 0
    v = v << 1;  //proximo bit vai pra posicao 7
  }
  
}

void mc(byte r, byte g, byte b) {
//mandando 3 bytes na ordem do datasheet
  mb(g);
  mb(r);
  mb(b);

}

int m_coord(int x, int y) { //coordenada -> fila
//função que recebe as coordenadas da matriz e transforma em posição na fila de 0 a 63
  bool l_par = 0; //declaramos esse bool que diz se a linha atual é par
  int x_real; //e esse int que nos dá o x real (a depender da paridade da linha atual)

  y % 2 == 0 ? l_par = true : l_par = false; //verifica o sentido da linha
  l_par ? x_real = x : x_real = 7 - x; //inverte o x para obter o x real a depender do sentido

  return (x_real + 8 * y);  //retorna a posição na fila

}

void m_organiza (int x, int y, byte r, byte g, byte b) { //cor na matriz
//recebe as coordenadas e as cores (bytes) e organiza na matriz permanente
  int c = m_coord(x, y); //pega a posição na fila com as coordenadas e salva em c
  //manda o r g b na posição certa
  m_perm[c][0] = r;
  m_perm[c][1] = g;
  m_perm[c][2] = b;

}

void m_limpar () {
//limpar a matriz de led

  for (int i = 0; i < 64; i++) { //apaga tudo da virtual de led
    m_perm[i][0] = 0;
    m_perm[i][1] = 0;
    m_perm[i][2] = 0;
  }

}

void m_mostra () { //manda a matriz virtual pra matriz real
//pega a matriz permanente virtual e mostra ela na matriz real
  noInterrupts();
  for (int i=0; i<64; i++) { //percorre todas as linhas e chama a funcão manda cor (mc) pra cada linha, e caso não tenha nada fica em 0,0,0
    mc(m_perm[i][0], m_perm[i][1], m_perm[i][2]);
  }
  interrupts();
}

// ----===---===---- FUNÇÕES DA COBRA ----===---===----

void c_start () {
//inicia a  matriz cobra com 3 tijolinhos na posição certa
  //c_perm[i][0] = X e c_perm[i][1] = Y; a cabeça é sempre a posição 0 e a bunda a última em c_tam - 1
  c_perm[0][0] = 4; c_perm[0][1] = 3; //cabeça em (4 e 3)
  c_perm[1][0] = 3; c_perm[1][1] = 3; //corpo à esquerda da cabeça, porque a cobra começa indo pra direita
  c_perm[2][0] = 2; c_perm[2][1] = 3; //bunda
  c_tam = 3; //agora a cobra tem 3 tijolinhos
}

int c_crescer (int x, int y) {
//confere se a posicao (x, y) da cabeça nova cai na fruta atual (f_perm[0]): devolve 1 se comeu e 0 se nao
  if (x == f_perm[0][0] && y == f_perm[0][1]) { //mesmo x && mesmo y então comeu
    return 1;
  }
  return 0;
}

void c_update (int dx, int dy) {
//anda um passo (recebendo a direção) considerando a possibilidade de comer
  //dx e dy valem -1, 0 ou +1 (quanto a cabeca anda em X e em Y)
  //na tela so a bunda (ultima posicao) some e reaparece uma casa a frente da cabeca
  int x_novo = (c_perm[0][0] + dx + 8) % 8; //a bunda vai pra frente da cabeca
  int y_novo = (c_perm[0][1] + dy + 8) % 8; //o "+ 8) % 8" faz a cobra atravessar a borda e sair do outro lado

  //codigo de comer: confere ANTES de arrastar, com a posicao nova da cabeca
  int comeu = c_crescer(x_novo, y_novo);
  if (comeu == 1 && c_tam < 64) { //o vetor so tem 64 posicoes
    c_tam++; //aumenta antes do for: ele comeca uma posicao mais pra frente e copia a bunda velha pra la, em vez de perder ela
  }

  //pra bunda continuar sendo a ultima posicao e a cabeca a 0, todo mundo desce uma posicao no vetor
  for (int i = c_tam - 1; i > 0; i--) { //da bunda pra cabeca e a bunda velha e sobrescrita
    c_perm[i][0] = c_perm[i - 1][0];
    c_perm[i][1] = c_perm[i - 1][1];
  }

  c_perm[0][0] = x_novo; //a antiga bunda vira a cabeca nova, na posicao 0
  c_perm[0][1] = y_novo;

  if (comeu == 1) {
    f_calcular(); //so depois de andar e crescer assim o sorteio ja enxerga a cobra inteira e a fruta nao nasce embaixo dela
  }

}

void c_andar () {
//processa a direcao do teclado e manda andar (passando a direção do teclado)
  switch (g_dir) { //cada direção vira um passo em X ou em Y
    case 'w': // sobe
      c_update(0, 1);
      break;
    case 's': // desce
      c_update(0, -1);
      break;
    case 'a': // esquerda
      c_update(-1, 0);
      break;
    case 'd': // direita
      c_update(1, 0);
      break;
  }

}

void c_aparecer () {
//codigo de mostrar a cobra onde apagamos a matriz de led virtual, pintamos a cobra+fruta nela e manda pra matriz real
  m_limpar();

  for (int i = 1; i < c_tam; i++) { //corpo é do tijolinho 1 ate a bunda (c_tam - 1)
    m_organiza(c_perm[i][0], c_perm[i][1], 0, 20, 0); //verde fraco
  }

  m_organiza(c_perm[0][0], c_perm[0][1], 20, 20, 0); //cabeça (posição 0) em amarelo pra saber pra onde ela vai

  f_aparecer(); //pinta a fruta na mesma matriz virtual, antes de mandar

  m_mostra(); //manda os 64 leds de uma vez pra matriz real
}

// ----===---===---- FUNÇÕES DA FRUTA ----===---===----

void f_sortear () {
//calcula uma posição aleatória

  //rand() devolve um numero de 0 a 32767 e o "% 8" pega o resto da divisao por 8, que so pode ser de 0 a 7
  f_perm[0][0] = rand() % 8; //x aleatorio dentro da matriz
  f_perm[0][1] = rand() % 8; //y aleatorio dentro da matriz

}

void f_arrastar () {
//arrasta o historico uma posicao pra baixo
  //a fruta mais velha é apagada e a posicao 0 fica livre pra fruta nova
  for (int i = 9; i > 0; i--) { //de baixo pra cima, igual a cobra, senao copia a mesma fruta em todas
    f_perm[i][0] = f_perm[i - 1][0];
    f_perm[i][1] = f_perm[i - 1][1];
  }
}

void f_calcular () {
//sorteia a fruta e enquanto ela cair embaixo da cobra, sorteia de novo
  f_arrastar(); //guarda a fruta atual no historico uma vez antes de sortear a nova 

  bool embaixo = true; //começa true so pra entrar no while

  while (embaixo) {
    f_sortear(); //sorteia x e y em f_perm[0]

    embaixo = false; //supõe que caiu num lugar livre...
    for (int i = 0; i < c_tam; i++) { //...e confere tijolinho por tijolinho, da cabeça (0) até a bunda (c_tam - 1)
      if (c_perm[i][0] == f_perm[0][0] && c_perm[i][1] == f_perm[0][1]) { //mesmo x && mesmo y então caiu em cima da cobra
        embaixo = true; //o while roda de novo caso esteja embaixo
        break;          //nao precisa olhar o resto da cobra e volta pra repetir o while
      }
    }
  }
}

void f_aparecer () {
//codigo de mostrar a fruta
    //onde so pinta a fruta atual (f_perm[0]) na matriz virtual, quem manda pra real e o c_aparecer
  m_organiza(f_perm[0][0], f_perm[0][1], 20, 0, 0); //vermelho fraco
}

// ----===---===---- FUNÇÕES DO JOGO ----===---===----

void j_dir () {
//pega a direcao no serial e joga pra variavel global
  if (Serial.available() > 0) { //confere se chegou alguma coisa no serial
    char tecla = Serial.read(); //pega o proximo caractere do serial
    //so aceita w a s d (o Serial Monitor tambem manda '\n' no fim da linha)
    //e ignora a direcao oposta, senao a cabeca entra no proprio corpo
    if ((tecla == 'w' && g_dir != 's') ||
        (tecla == 's' && g_dir != 'w') ||
        (tecla == 'a' && g_dir != 'd') ||
        (tecla == 'd' && g_dir != 'a')) { //qualquer um dos 4 pares valido: a tecla ja e a propria direcao
      g_dir = tecla;
    }
  }
}

void j_start () {
//inicia o jogo: monta a cobra, sorteia a primeira fruta e mostra as duas na matriz
  g_dir = 'd';  //a cobra comeca indo pra direita (o c_start monta ela virada pra la)
  c_start();    //cobra de 3 tijolinhos
  f_calcular(); //primeira fruta, ja fora da cobra (depois do c_start, senao o while confere uma cobra vazia)
  c_aparecer(); //primeira imagem na matriz real
}

void j_over () {
//confere o tamanho da cobra: se ela encheu a matriz (64), o jogo acaba e tudo para (HALT)
  if (c_tam >= 64) {
    noInterrupts(); //desliga as interrupcoes: o Timer1 para de chamar o j_dir, ninguem mais mexe em nada
    while (true) {} //trava o processador aqui pra sempre (a matriz continua mostrando a ultima imagem, porque o led guarda a cor sozinho)
  }
}

// ----===---===---- SETUP E LOOP ----===---===----

void setup() {
  
  Serial.begin(9600); //inicia o serial

  pinMode(PINO_IN, OUTPUT);   //o arduino manda o sinal pro DIN
  digitalWrite(PINO_IN, LOW); //fio parado em LOW

  srand(analogRead(A0)); //semente do rand: o ruido do pino A0 solto muda a cada boot (sem isso a fruta sai sempre na mesma sequencia)

  // ---- interrupcao (Timer2 chama o j_dir e Timer1 faz o jogo andar; o Uno nao tem Timer3) ----
  noInterrupts();

  //Timer2 (8 bits, conta ate 255) em modo CTC conta de 0 ate OCR2A gera a interrupcao e zera sozinho
  TCCR2A = (1 << WGM21);                              //WGM21 = CTC, nada de PWM nos pinos do timer
  TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);   //CS22 + CS21 + CS20 = prescaler 1024 no Timer2 (16 MHz / 1024 = 15625 contagens por segundo)
  OCR2A = 155;                                        //156 contagens (0 a 155) / 15625 = ~10 ms, ou 100 vezes por segundo (cabe nos 8 bits)
  TIMSK2 = (1 << OCIE2A);                             //liga a interrupcao de comparacao com o OCR2A

  //Timer1 (16 bits) no mesmo modo CTC, so que mais devagar: e o passo da cobra (4687 nao cabe nos 8 bits do Timer2)
  TCCR1A = 0;                                         //nada de PWM nos pinos do timer
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);  //WGM12 = CTC; CS12 + CS10 = prescaler 1024 (15625 contagens por segundo)
  OCR1A = 4687;                                       //4688 contagens / 15625 = ~300 ms por passo (diminuir aqui deixa a cobra mais rapida)
  TIMSK1 = (1 << OCIE1A);                             //liga a interrupcao de comparacao com o OCR1A
  interrupts();
  // ---- fim da interrupcao ----

  j_start(); //inicia o jogo (fora do bloco noInterrupts, porque o m_mostra liga as interrupcoes de volta no fim; e depois do srand, pra primeira fruta ja sair aleatoria)

}

ISR(TIMER2_COMPA_vect) { //roda sozinha a cada 10 ms
  j_dir(); //le a tecla e a 9600 chega no maximo 1 caractere por ms e o buffer do serial guarda 64, entao 10 ms sobra
}

ISR(TIMER1_COMPA_vect) { //roda sozinha a cada 300 ms: um passo do jogo
  c_andar();    //anda (e se comeu, cresce e sorteia outra fruta)
  c_aparecer(); //mostra a imagem nova
  j_over();     //depois de mostrar: se encheu, a matriz fica com a cobra cheia e tudo para
}

void loop() {
}
