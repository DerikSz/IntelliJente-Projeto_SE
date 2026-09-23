// ============================================================================
//  PROJETO: Segurança Residencial com ESP32-S3-N16R8 + RFID + Blynk
// ============================================================================

// ======================== BLYNK =============================================

#define BLYNK_TEMPLATE_ID "TMPL2vCK3JGi6"
#define BLYNK_TEMPLATE_NAME "testLed"
#define BLYNK_AUTH_TOKEN "OS4CMvnheFSqPny83QX7UUtTKAZldyrG"

// ======================== RFID (HSPI - pinos fixos no S3) ==================
// MOSI=11, MISO=13, SCK=12, SS=10

#define SS_PIN 10
#define RST_PIN 5

// ======================== PINOS DOS LEDs ====================================
// Evitados: 0 (boot), 19-20 (USB), 26-37 (flash/PSRAM), 43-44 (Serial), 46 (input only)

#define LED_VERM 40  // LED vermelho (alarme / acesso negado)
#define LED_VERD 14  // LED verde   (acesso liberado)
#define LED_SALA 15
#define LED_QUARTO 16
#define LED_BANHEIRO 17
#define LED_COZINHA 18
#define LED_QUARTO2 21
#define LED_POSTE 42  // LED externo (controle automático pelo sensor)

// ======================== SENSORES ==========================================

#define SENSOR_PIN 4   // Fotoresistor (ADC1_CH3)
#define SENSOR_MOV 7   // Sensor PIR HC-SR501
#define THRESHOLD 500  // Limite: abaixo disso, está escuro

// ======================== BUZZER ============================================

#define BUZZER 39

// ======================== BIBLIOTECAS =======================================

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <SPI.h>
#include <MFRC522.h>

// ======================== WI-FI =============================================

char ssid[] = "moto g(9) power Gui";
char pass[] = "gui951458738";

// ======================== VARIÁVEIS =========================================

MFRC522 mfrc522(SS_PIN, RST_PIN);

bool estadoLED = false;  // estado do alarme: false = desligado, true = ligado
bool alarmeEst = false;  // controle interno do alarme

String UID_AUTORIZADO = "UID_ DO_RFID";  // UID da tag autorizada

// ======================== SETUP =============================================

void setup() {
  Serial.begin(115200);
  Serial.println("\nConectando ao WiFi...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID iniciado!");
  Serial.println("Aproxime uma tag...");

  pinMode(LED_VERM, OUTPUT);
  pinMode(LED_VERD, OUTPUT);
  pinMode(LED_SALA, OUTPUT);
  pinMode(LED_QUARTO, OUTPUT);
  pinMode(LED_BANHEIRO, OUTPUT);
  pinMode(LED_COZINHA, OUTPUT);
  pinMode(LED_QUARTO2, OUTPUT);
  pinMode(LED_POSTE, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(SENSOR_MOV, INPUT);

  digitalWrite(LED_VERM, LOW);
  digitalWrite(LED_VERD, LOW);
  digitalWrite(LED_POSTE, LOW);
}

// ======================== LOOP ==============================================

void loop() {
  Blynk.run();

  int luminosidade = analogRead(SENSOR_PIN);
  // Serial.print("Sensor: ");
  // Serial.println(luminosidade);

  if (luminosidade < THRESHOLD) {
    digitalWrite(LED_POSTE, HIGH);
  } else {
    digitalWrite(LED_POSTE, LOW);
  }

  if (alarmeEst && digitalRead(SENSOR_MOV) == HIGH) {
    Serial.println("Movimento detectado! Alarme acionado...");
    Blynk.logEvent("movimento_detectado");
    alarme(true);
    estadoLED = true;
    aplicarEstado(true);
    delay(3000);
  }

  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  RFID();
}

// ======================== ALARME ============================================

void alarme(bool ativar) {
  if (ativar) {
    tone(BUZZER, 300, 100);
  } else {
    noTone(BUZZER);
  }
}

// ======================== APLICA ESTADO DO ALARME ===========================

void aplicarEstado(bool espelhaNoApp) {
  if (estadoLED) {
    digitalWrite(LED_VERM, HIGH);
  } else {
    digitalWrite(LED_VERM, LOW);
  }

  if (espelhaNoApp) {
    Blynk.virtualWrite(V0, estadoLED);
  }

  Serial.println(estadoLED ? "ALARME ACIONADO" : "ALARME DESLIGADO");
}

// ======================== FEEDBACK LED RFID ================================

void feedbackRFID(bool autorizado) {
  if (autorizado) {
    digitalWrite(LED_VERD, HIGH);
  } else {
    digitalWrite(LED_VERM, HIGH);
  }

  delay(1500);

  digitalWrite(LED_VERD, LOW);
  digitalWrite(LED_VERM, LOW);
}

// ======================== LER RFID ==========================================

void RFID() {
  String uid = "";

  Serial.println("UID TAG:");

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uid.concat(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    uid.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  uid.toUpperCase();
  Serial.println(uid);

  if (uid.equals(UID_AUTORIZADO)) {
    Serial.println("Tag autorizada! Alarme desligado...");
    Blynk.logEvent("alarme_desligado");
    feedbackRFID(true);
    alarmeEst = false;
    alarme(false);
    estadoLED = false;
    aplicarEstado(true);
  } else {
    Serial.println("Tag não autorizada! Alarme acionado...");
    Blynk.logEvent("alarme_acionado");
    feedbackRFID(false);
    alarmeEst = true;
    alarme(true);
    estadoLED = true;
    aplicarEstado(true);
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

// ======================== BLYNK: BOTÃO ALARME (V0) =========================

BLYNK_WRITE(V0) {
  int valor = param.asInt();

  if (valor == 1) {
    estadoLED = true;
    alarmeEst = true;
    aplicarEstado(true);
    Serial.println("Alarme ligado pelo app");
  } else {
    Blynk.virtualWrite(V0, 1);
  }
}

// ======================== BLYNK: SENHA (V2) ================================

BLYNK_WRITE(V2) {
  int senha = param.asInt();

  if (senha == 1234 && alarmeEst) {
    Serial.println("Senha correta! Alarme desligado...");
    Blynk.logEvent("senha_correta");
    feedbackRFID(true);
    alarmeEst = false;
    alarme(false);
    estadoLED = false;
    aplicarEstado(true);
  } else {
    Serial.println("Senha incorreta!");
    Blynk.logEvent("senha_incorreta");
  }
}

// ======================== BLYNK: SLIDER LUMINOSIDADE (V3) ==================

BLYNK_WRITE(V3) {
  int brilho = param.asInt();

  analogWrite(LED_SALA, brilho);
  analogWrite(LED_QUARTO, brilho);
  analogWrite(LED_BANHEIRO, brilho);
  analogWrite(LED_COZINHA, brilho);
  analogWrite(LED_QUARTO2, brilho);

  Serial.print("Brilho geral: ");
  Serial.println(brilho);
}

// ======================== BLYNK: BOTÃO SALA (V4) ==========================

BLYNK_WRITE(V4) {
  int estado = param.asInt();
  digitalWrite(LED_SALA, estado);
  Serial.println(estado ? "Sala LIGADA" : "Sala desligada");
}

// ======================== BLYNK: BOTÃO QUARTO (V5) ========================

BLYNK_WRITE(V5) {
  int estado = param.asInt();
  digitalWrite(LED_QUARTO, estado);
  Serial.println(estado ? "Quarto LIGADO" : "Quarto desligado");
}

// ======================== BLYNK: BOTÃO QUARTO2 (V6) =======================

BLYNK_WRITE(V6) {
  int estado = param.asInt();
  digitalWrite(LED_QUARTO2, estado);
  Serial.println(estado ? "Quarto2 LIGADO" : "Quarto2 desligado");
}

// ======================== BLYNK: BOTÃO BANHEIRO (V7) ======================

BLYNK_WRITE(V7) {
  int estado = param.asInt();
  digitalWrite(LED_BANHEIRO, estado);
  Serial.println(estado ? "Banheiro LIGADO" : "Banheiro desligado");
}

// ======================== BLYNK: BOTÃO COZINHA (V8) =======================

BLYNK_WRITE(V8) {
  int estado = param.asInt();
  digitalWrite(LED_COZINHA, estado);
  Serial.println(estado ? "Cozinha LIGADA" : "Cozinha desligada");
}
