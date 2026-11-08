# SEI depois da AP1

Códigos de Arduino de Sistemas Embarcados e Internet das Coisas (2026.2) feitos depois da AP1: o sábado letivo de 03/10, as aulas de interrupção com o Timer1 e a matriz de LED WS2812B. Cada pasta é um sketch que abre direto na Arduino IDE (o nome da pasta é o nome do `.ino`).

Todos compilam para o Arduino Uno.

## 01. Sábado letivo (03/10)

| sketch | o que faz |
|---|---|
| `casa_display` | escreve texto no display de 4 dígitos do shield com a função `pd(texto, digitos)` e uma tabela ASCII indexada pelo código do caractere (do espaço ao `z`); anima `CASA` entrando e saindo com `millis` |
| `timer1_300ms` | primeiro contato com interrupção: Timer1 em modo CTC, prescaler 256, `OCR1A = 18749`, dispara a cada 300 ms; a `ISR` só levanta uma flag e o `loop` executa a rotina |

## 02. Interrupções (06/10 a 27/10)

| sketch | o que faz |
|---|---|
| `timer1_300ms` | versão final do exemplo do Timer1 |
| `casa_display_timer1` | a animação da `CASA` passa a andar pelo Timer1 de 300 ms, com `switch (cont)` trocando o quadro; a `pd` recebe os dígitos em nibble (`pd("A", B0100)`) e a função `shot` substitui o `shiftOut`, mandando o número de bits que se pedir |
| `contador_botoes` | contador de -999 a 9999 com os botões S1 (soma), S2 (subtrai) e S3 (zera); a varredura do display e a leitura dos botões rodam na interrupção do Timer1 a cada 2 ms, com debounce de 50 ms |

### Shield multifunção (HW 262)

Dois 74HC595 em cascata e display de sete segmentos de 4 dígitos, ânodo comum: o segmento acende com bit 0, e o primeiro byte enviado é a letra, o segundo escolhe o dígito.

| sinal | pino |
|---|---|
| latch | 4 |
| clock | 7 |
| data | 8 |
| botões S1, S2, S3 | A1, A2, A3 |

## 03. Matriz de LED WS2812B (03/11 a 06/11)

Matriz 8x8 de WS2812B com o DIN no pino 6.

| sketch | o que faz |
|---|---|
| `matriz_texto` | escreve G, A e Y na matriz |
| `matriz_pride` | anima bandeiras, coração e arco-íris |
| `matriz_teclado` | imprime na matriz a letra digitada no Serial Monitor (G, A ou Y); os comentários explicam o protocolo do WS2812B, tempos de bit e reset |
| `matriz_desenhos` | teclas 1, 2 e 3 mostram raio, triângulo e coração por 1 s, com a escolha tratada na interrupção do Timer1 |
| `cobrinha_matriz` | jogo da cobrinha **sem biblioteca**: o sinal do WS2812B sai escrito direto no `PORTD`, a direção (w, a, s, d pela serial) é lida na interrupção do Timer2 e o passo do jogo roda no Timer1 a cada 300 ms. `ROTEIRO.txt` é o esqueleto comentado usado para montar o código |

Os quatro primeiros usam a biblioteca **Adafruit NeoPixel** (Gerenciador de Bibliotecas da IDE). A cobrinha não usa nenhuma.

A cobrinha ainda está em andamento: compila, mas falta colisão com o próprio corpo e o fim de jogo em 64 precisa ser chamado antes de sortear a fruta nova. Para mandar a direção sem apertar Enter, use um terminal serial em 9600 em vez do Serial Monitor da IDE.

## Compilar pela linha de comando

```bash
arduino-cli compile --fqbn arduino:avr:uno 02-interrupcoes/contador_botoes
```
