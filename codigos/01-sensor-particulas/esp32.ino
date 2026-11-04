/*
 * ESP32 - Sistema de Monitoramento de Sustentabilidade Têxtil v2.0
 *
 * Sensores:
 *   - GP2Y1010AU0F -> Partículas liberadas (ppm)
 *   - Sensor de Densidade (célula de carga + deslocamento)
 *
 * O ESP32 envia para o servidor:
 *   - Quantidade estimada de partículas liberadas
 *   - Densidade inicial e final (diferença antes/depois do desgaste)
 *   - O servidor calcula a duração estimada em anos
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* SSID = "SEU_WIFI_SSID";
const char* PASSWORD = "SUA_SENHA_WIFI";
const char* SERVER_URL = "http://192.168.x.x:3000/api/esp32/dados";

#define PM_PIN A0
#define PM_LED_PIN 5

int tipoTecidoAtual = 1; // 1=sintetico, 2=natural, 3=misto
unsigned long lastSend = 0;
const unsigned long INTERVAL = 5000;

float densidadeInicial = 0;

void connectWiFi() {
  Serial.print("Conectando ao WiFi");
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  Serial.println("Inicializando ESP32 - Sustentabilidade Textil v2.0...");

  pinMode(PM_LED_PIN, OUTPUT);
  pinMode(PM_PIN, INPUT);

  // Define densidade inicial conforme o tipo de tecido
  switch (tipoTecidoAtual) {
    case 1: densidadeInicial = 1.38; break; // sintetico
    case 2: densidadeInicial = 1.52; break; // natural
    case 3: densidadeInicial = 1.45; break; // misto
  }

  connectWiFi();
}

float readParticulateMatter() {
  digitalWrite(PM_LED_PIN, LOW);
  delayMicroseconds(280);
  float vo = analogRead(PM_PIN) * (3.3 / 4095.0);
  delayMicroseconds(40);
  digitalWrite(PM_LED_PIN, HIGH);
  delayMicroseconds(9680);

  float dustDensity = 0.17 * vo - 0.1;
  if (dustDensity < 0) dustDensity = 0;
  return dustDensity * 100;
}

float readDensity() {
  // Simula leitura de densidade após desgaste
  // Em um projeto real, usar célula de carga + sensor de deslocamento
  float variacao = random(1, 50) / 1000.0;
  return densidadeInicial + variacao;
}

String getTipoString() {
  switch (tipoTecidoAtual) {
    case 1: return "sintetico";
    case 2: return "natural";
    case 3: return "misto";
    default: return "natural";
  }
}

void sendData() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado!");
    connectWiFi();
    return;
  }

  HTTPClient http;
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/json");

  float particulas = readParticulateMatter();
  float densidadeFinal = readDensity();

  StaticJsonDocument<256> doc;
  doc["tipo"] = getTipoString();
  doc["particulas_liberadas"] = (int)particulas;
  doc["densidade_inicial"] = densidadeInicial;
  doc["densidade_final"] = densidadeFinal;

  String jsonString;
  serializeJson(doc, jsonString);

  Serial.println("Enviando dados:");
  Serial.println(jsonString);

  int httpResponseCode = http.POST(jsonString);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("Resposta servidor: ");
    Serial.println(response);
  } else {
    Serial.print("Erro no envio: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

void loop() {
  if (millis() - lastSend >= INTERVAL) {
    lastSend = millis();
    sendData();
  }

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "1") { tipoTecidoAtual = 1; densidadeInicial = 1.38; Serial.println("Tipo: Sintetico"); }
    else if (cmd == "2") { tipoTecidoAtual = 2; densidadeInicial = 1.52; Serial.println("Tipo: Natural"); }
    else if (cmd == "3") { tipoTecidoAtual = 3; densidadeInicial = 1.45; Serial.println("Tipo: Misto"); }
  }

  delay(100);
}
