#ifdef ESP8266

#include <ESP8266WiFi.h>

#else

#include <WiFi.h>

#endif

#include <ModbusIP_ESP8266.h>

const int FornoLigado = 3;
const int Temperatura = 2;
const int SetpointForno = 2;

const int id3Liberado = 3;
const int senha3 = 3333;

const int solAcesso3 = 2;

const int passCode3 = 102;
const int idCode3 = 103;

const int pinoTemperatura = 34;

const int LedAcesso3 = 6;
const int ESPConectado = 7;

ModbusIP mb;

int temperaturaAtual = 0;
int setpoint = 25;

bool fornoLigado = 0;
bool acessoLiberado = 1;

int SSPAutorizado = 0;

bool estadoAcesso3 = 0;

int codigo3 = 0;
int idRecebido3 = 0;

void setup() {

  Serial.begin(115200);

  // Preencha com os dados da sua rede antes de gravar no ESP32
  WiFi.begin("SEU_WIFI_SSID", "SUA_SENHA_WIFI");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println(WiFi.localIP());

  mb.server();

  mb.addIreg(Temperatura);
  mb.addIsts(FornoLigado);

  mb.addCoil(solAcesso3);

  mb.addIsts(LedAcesso3);
  mb.addIsts(ESPConectado);

  mb.Ists(ESPConectado, true);

  mb.addHreg(SetpointForno, 25);

  mb.addHreg(passCode3, 0);
  mb.addHreg(idCode3, 0);
}

void loop() {

  mb.task();

  int leitura = analogRead(pinoTemperatura);

  temperaturaAtual = map(leitura, 0, 4095, 0, 100);
  temperaturaAtual = constrain(temperaturaAtual, 0, 100);

  mb.Ireg(Temperatura, temperaturaAtual);

  estadoAcesso3 = mb.Coil(solAcesso3);

  codigo3 = mb.Hreg(passCode3);

  idRecebido3 = mb.Hreg(idCode3);

  if (acessoLiberado == 1) {

    if (
      estadoAcesso3 == 1 &&
      idRecebido3 == id3Liberado &&
      codigo3 == senha3
    ) {

      acessoLiberado = 0;
      SSPAutorizado = id3Liberado;

      Serial.println("SSP 3 autorizado");
    }
  }

  else {

    if (
      SSPAutorizado == id3Liberado &&
      estadoAcesso3 == 0
    ) {

      acessoLiberado = 1;
      SSPAutorizado = 0;

      Serial.println("Acesso do SSP 3 liberado");
    }
  }

  mb.Ists(
    LedAcesso3,
    SSPAutorizado == id3Liberado
  );

  if (SSPAutorizado == id3Liberado) {
    setpoint = mb.Hreg(SetpointForno);
  }

  if (temperaturaAtual < setpoint) {
    fornoLigado = 1;
  }

  else {
    fornoLigado = 0;
  }

  mb.Ists(FornoLigado, fornoLigado);

  Serial.print("Temperatura: ");
  Serial.print(temperaturaAtual);

  Serial.print(" | Setpoint: ");
  Serial.print(setpoint);

  Serial.print(" | ID recebido: ");
  Serial.print(idRecebido3);

  Serial.print(" | Senha recebida: ");
  Serial.print(codigo3);

  Serial.print(" | SSP autorizado: ");
  Serial.print(SSPAutorizado);

  Serial.print(" | Acesso liberado: ");
  Serial.print(acessoLiberado);

  Serial.print(" | Forno: ");
  Serial.println(fornoLigado);

  delay(500);
}

//SENHA CERTA
//ID CERTO
// BOTAO DE SOLICITAR ACESSO = 0 
// AÍ PERDE O ACESSO