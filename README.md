# IntelliJente-Projeto_SE
Projeto desenvolvido com o intuito de cumprir com a obrigação de nossa atividade de SE do professor Carlos Alberto :)

Protótipo de automação e segurança residencial construído com ESP32-S3, controlado pelo aplicativo Blynk. O projeto combina controle de iluminação por cômodo, iluminação externa automática por luminosidade e um alarme com sensor de movimento, que é armado pelo app e desarmado por cartão RFID ou senha.

<img width="1374" height="600" alt="image" src="https://github.com/user-attachments/assets/56b3b1b3-aad6-4f1b-8371-4b395d529838" />

## Funcionalidades

| Recurso | Descrição |
|---|---|
| Iluminação por cômodo | Sala, quarto, quarto 2, banheiro e cozinha ligados e desligados individualmente pelo app |
| Brilho geral | Um slider controla a intensidade (PWM) dos cinco LEDs internos ao mesmo tempo |
| Poste automático | Um fotoresistor acende o LED do poste quando o ambiente fica escuro |
| Alarme | Armado pelo app. Com o alarme armado, o sensor PIR dispara buzzer e notificação ao detectar movimento |
| Desarme | Somente por tag RFID autorizada ou senha digitada no app. O botão do app não consegue desarmar |
| Notificações | Eventos do Blynk para movimento, alarme acionado, alarme desligado e senha certa ou errada |

## Hardware

| Componente | Função |
|---|---|
| ESP32-S3 (N16R8) | Microcontrolador, Wi-Fi e conexão com o Blynk |
| Módulo RFID RC522 | Leitura de tags para desarmar o alarme |
| Sensor PIR HC-SR501 | Detecção de movimento |
| Fotoresistor (LDR) | Medição de luminosidade para o poste |
| Buzzer | Aviso sonoro do alarme |
| 7 LEDs + resistores | Cinco cômodos, poste e um LED verde de acesso |
| LED vermelho + resistor | Estado do alarme e acesso negado |
| Protoboard e jumpers | Montagem |

### Pinagem

| Função | GPIO |
|---|---|
| RFID SS | 10 |
| RFID RST | 5 |
| RFID MOSI / SCK / MISO | 11 / 12 / 13 |
| LED vermelho (alarme) | 40 |
| LED verde (acesso liberado) | 14 |
| LED sala | 15 |
| LED quarto | 16 |
| LED banheiro | 17 |
| LED cozinha | 18 |
| LED quarto 2 | 21 |
| LED poste | 42 |
| Fotoresistor (ADC) | 4 |
| Sensor PIR | 7 |
| Buzzer | 39 |

Os GPIOs 0, 19, 20, 26 a 37, 43, 44 e 46 foram evitados de propósito, pois têm função especial no ESP32-S3 (boot, USB, flash/PSRAM, serial).

## Configuração do Blynk

Crie um template no Blynk Cloud com os datastreams abaixo e os eventos da segunda tabela.

| Pino virtual | Widget sugerido | Tipo | Função |
|---|---|---|---|
| V0 | Botão (switch) | Inteiro 0 a 1 | Arma o alarme e mostra seu estado |
| V2 | Campo numérico | Inteiro | Senha para desarmar |
| V3 | Slider | Inteiro 0 a 255 | Brilho dos LEDs internos |
| V4 | Botão | Inteiro 0 a 1 | LED da sala |
| V5 | Botão | Inteiro 0 a 1 | LED do quarto |
| V6 | Botão | Inteiro 0 a 1 | LED do quarto 2 |
| V7 | Botão | Inteiro 0 a 1 | LED do banheiro |
| V8 | Botão | Inteiro 0 a 1 | LED da cozinha |

## Como usar

1. Instale a Arduino IDE e o pacote de placas **esp32** (Espressif).
2. Instale as bibliotecas **Blynk** e **MFRC522** pelo Gerenciador de Bibliotecas.
3. Selecione a placa **ESP32S3 Dev Module** e a porta serial correta.
4. No topo do sketch, preencha `BLYNK_TEMPLATE_ID`, `BLYNK_TEMPLATE_NAME` e `BLYNK_AUTH_TOKEN` com os dados do seu template, e `ssid` e `pass` com sua rede Wi-Fi.
5. Descubra o UID da sua tag: grave o sketch, abra o Monitor Serial (115200), aproxime a tag e copie o UID impresso.
6. Cole esse UID em `UID_AUTORIZADO`, exatamente como foi impresso (maiúsculas, bytes separados por espaço, incluindo o espaço inicial).
7. Grave novamente e teste pelo app.

> Nunca publique o token do Blynk nem a senha do Wi-Fi. Mantenha essas credenciais em um arquivo `secrets.h` listado no `.gitignore`, ou substitua elas por placeholders antes de commitar.

## Como o alarme funciona

O alarme só reage ao PIR quando está armado. Ele é armado pelo botão V0 do app (ou por uma tag RFID não autorizada) e é desarmado por uma tag autorizada ou pela senha em V2. Se o botão V0 for desligado no app, o sketch o devolve para ligado, de modo que o alarme só sai desse estado por RFID ou senha. A explicação detalhada de cada trecho do código está em [EXPLICACAO.md](EXPLICACAO.md).

## Limitações conhecidas

O buzzer emite um único beep curto (100 ms) a cada disparo, não uma sirene contínua. A conexão inicial com o Blynk é bloqueante, então sem Wi-Fi o sketch não sai do `setup()`. Há chamadas de `delay()` que pausam o loop por até 3 segundos, e a senha (`1234`) está fixa no código. Esses pontos são candidatos naturais a melhorias.

## Tecnologias

C++ (Arduino framework), ESP32-S3, Blynk IoT, biblioteca MFRC522, protocolo SPI.
