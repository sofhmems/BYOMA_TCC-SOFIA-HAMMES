#include <SparkFunTSL2561.h>
#include <Wire.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"




// ------------------------------------------------------------
// CREDENCIAIS DE ACESSO (troque antes de gravar no ESP32)
// ------------------------------------------------------------
const char *USUARIO_SISTEMA = "SEU_USUARIO";
const char *SENHA_SISTEMA = "SUA_SENHA";

// ------------------------------------------------------------
// PINOS
// ------------------------------------------------------------
#define PINO_PASSO       32    // Motor de passo - pino de passo (STEP)
#define PINO_DIRECAO     33    // Motor de passo - pino de direcao (DIR)
#define RELE_DESGASTE    25    // Rele do motor de desgaste (lixa)
#define ESTEIRA_IN1      14    // Rele da esteira - sentido 1
#define ESTEIRA_IN2      26    // Rele da esteira - sentido 2
#define LED_POEIRA       ((gpio_num_t)27) // LED do sensor de particulas
#define LED              18    // LED da camara escura (liga so na esteira ate a camara escura)




// Logica do rele da LIXA: este modulo liga com LOW e desliga com HIGH
#define RELE_LIGADO      LOW
#define RELE_DESLIGADO   HIGH




// Sentido do motor de passo
#define SENTIDO_DESCE    HIGH
#define SENTIDO_SOBE     LOW




// Saida Vo do sensor de particulas = GPIO34 = ADC1_CHANNEL_6
#define CANAL_ADC        ADC1_CHANNEL_6
#define VREF_PADRAO      1100
#define NUMERO_AMOSTRAS  64




// ------------------------------------------------------------
// CALIBRACAO SENSOR DE PARTICULAS
// ------------------------------------------------------------
#define TENSAO_BASE      600.0 // tensao em ambiente limpo (mV)
#define SENSIBILIDADE    0.2   // 0.5 V -> 0.1 mg/m3




// ------------------------------------------------------------
// TEMPOS DE OPERACAO (ms)
// ------------------------------------------------------------
#define TEMPO_ESTEIRA_1      9500
#define TEMPO_ESTEIRA_2      13000
#define TEMPO_ESTEIRA_3      13000


// Motor de passo + lixa (tempos do codigo de teste)
#define TEMPO_PASSO_SOBE_INICIAL_MS 2000 // subida inicial (logo apos o SIM)
#define TEMPO_ESPERA_CIMA_MS        1000 // parado em cima
#define TEMPO_PASSO_DESCE_MS        2000 // descida
#define TEMPO_DESGASTE              1500 // lixa ligada (ms)
#define TEMPO_PASSO_SOBE_MS         2000 // subida final (volta ao lugar)
#define VELOCIDADE_PASSO_US         1500 // meio periodo do pulso STEP (us)


#define TEMPO_PAUSA_MS       500   // pausa entre movimentos
#define TEMPO_MEDICAO_POEIRA TEMPO_DESGASTE // mede durante a lixa
#define INTERVALO_AMOSTRA_MS 100   // intervalo entre amostras de particulas




// ------------------------------------------------------------
// OBJETOS E VARIAVEIS GLOBAIS
// ------------------------------------------------------------
SFE_TSL2561 sensorLuz;
boolean ganho;                   // false = X1, true = X16
unsigned int tempoIntegracao;    // tempo de integracao do TSL2561 (ms)




static esp_adc_cal_characteristics_t *calibracaoAdc = NULL;




double luxAntes = -1.0;          // luminosidade antes do desgaste
double luxDepois = -1.0;         // luminosidade depois do desgaste
float particulasMedidas = -1.0;  // media de particulas medida
double somaParticulas = 0.0;
unsigned int quantidadeAmostras = 0;




// ============================================================
// LOG COM TEMPO DESDE A PARTIDA
// ============================================================
void registrar(const char *msg) {
  Serial.print("[");
  Serial.print(millis());
  Serial.print(" ms] ");
  Serial.println(msg);
}




// ============================================================
// FUNCOES DE APOIO - LED DA CAMARA ESCURA
// ============================================================
void ledLigado() {
  digitalWrite(LED, HIGH);
}




void ledDesligado() {
  digitalWrite(LED, LOW);
}




// ============================================================
// FUNCOES DE APOIO - LUMINOSIDADE
// ============================================================
void iniciarLuminosidade() {
  sensorLuz.begin();
 
  unsigned char id;
  if (sensorLuz.getID(id)) {
    Serial.print("TSL2561 - ID: 0X");
    Serial.print(id, HEX);
    Serial.println(" (esperado 0X5X)");
  } else {
    byte erro = sensorLuz.getError();
    imprimirErro(erro);
  }




  // Ganho baixo (1X) e integracao de 402 ms
  ganho = 0;
  unsigned char codigoTempo = 2;
  sensorLuz.setTiming(ganho, codigoTempo, tempoIntegracao);
  sensorLuz.setPowerUp();
}




double medirLuminosidade() {
  unsigned int dado0, dado1;
  double valorLux = -1.0;
  boolean leituraValida = false;




  delay(tempoIntegracao); // espera a integracao terminar




  if (sensorLuz.getData(dado0, dado1)) {
    Serial.print(" dado0: ");
    Serial.print(dado0);
    Serial.print(" dado1: ");
    Serial.println(dado1);




    leituraValida = sensorLuz.getLux(ganho, tempoIntegracao, dado0, dado1, valorLux);
    if (!leituraValida) {
      Serial.println("Aviso: sensor de luz saturado (leitura ruim).");
    }
  } else {
    byte erro = sensorLuz.getError();
    imprimirErro(erro);
  }




  return valorLux;
}




// ============================================================
// FUNCOES DE APOIO - MOTOR DE PASSO (NEMA)
// ============================================================
// Gira o motor de passo pelo tempo (ms) e sentido escolhidos
// SENTIDO_DESCE (HIGH) = desce o motor de desgaste / SENTIDO_SOBE (LOW) = sobe
void girarPasso(boolean sentido, unsigned long tempoMs) {
  // GARANTE que a lixa fique desligada durante todo o movimento
  desgasteDesligado();




  digitalWrite(PINO_DIRECAO, sentido);
  delay(50);




  if (sentido == SENTIDO_DESCE) {
    registrar("Motor de passo descendo");
  } else {
    registrar("Motor de passo subindo");
  }




  unsigned long inicioMs = millis();
  while (millis() - inicioMs < tempoMs) {
    digitalWrite(PINO_PASSO, HIGH);
    delayMicroseconds(VELOCIDADE_PASSO_US);
    digitalWrite(PINO_PASSO, LOW);
    delayMicroseconds(VELOCIDADE_PASSO_US);




    // GARANTE novamente que a lixa continue desligada
    desgasteDesligado();
  }




  // NEMA PAROU
  digitalWrite(PINO_PASSO, LOW);




  // GARANTE que a lixa esteja desligada antes de qualquer outra coisa
  desgasteDesligado();




  registrar("Motor de passo PAROU - lixa ainda desligada");
}




// ============================================================
// FUNCOES DE APOIO - ESTEIRA (RELE DE 2 CANAIS: IN1 / IN2)
// ============================================================
// Cada funcao desliga o lado oposto ANTES de ligar o seu,
// assim os dois sentidos nunca ficam ligados ao mesmo tempo.
void esteiraDesligada() {
  digitalWrite(ESTEIRA_IN1, LOW);
  digitalWrite(ESTEIRA_IN2, LOW);
}




// SENTIDOS INVERTIDOS:
// Sentido 1 passa a acionar ESTEIRA_IN2 e Sentido 2 passa a acionar ESTEIRA_IN1
void esteiraSentido1() {
  digitalWrite(ESTEIRA_IN1, LOW);
  digitalWrite(ESTEIRA_IN2, HIGH);
}




void esteiraSentido2() {
  digitalWrite(ESTEIRA_IN2, LOW);
  digitalWrite(ESTEIRA_IN1, HIGH);
}




// ============================================================
// FUNCOES DE APOIO - RELE DA LIXA
// ============================================================
void desgasteLigado() {
  digitalWrite(RELE_DESGASTE, RELE_LIGADO);
}




void desgasteDesligado() {
  digitalWrite(RELE_DESGASTE, RELE_DESLIGADO);
}




// ============================================================
// FUNCOES DE APOIO - SENSOR DE PARTICULAS
// ============================================================
void iniciarSensorParticulas() {
  gpio_reset_pin(LED_POEIRA);
  gpio_set_direction(LED_POEIRA, GPIO_MODE_OUTPUT);
  gpio_set_level(LED_POEIRA, 0);




  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(CANAL_ADC, ADC_ATTEN_DB_11);




  calibracaoAdc = (esp_adc_cal_characteristics_t *)calloc(
      1, sizeof(esp_adc_cal_characteristics_t));




  if (calibracaoAdc == NULL) {
    Serial.println("Falha ao alocar calibracao do ADC.");
    return;
  }




  esp_adc_cal_characterize(
      ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, VREF_PADRAO, calibracaoAdc);
}




float lerParticulas() {
  uint32_t leituraAdc = 0;
  for (int i = 0; i < NUMERO_AMOSTRAS; i++) {
    leituraAdc += adc1_get_raw(CANAL_ADC);
  }
  leituraAdc /= NUMERO_AMOSTRAS;




  uint32_t tensaoMv = esp_adc_cal_raw_to_voltage(leituraAdc, calibracaoAdc);
  float tensao = tensaoMv / 1000.0;




  float particulas = (tensao - (TENSAO_BASE / 1000.0)) * SENSIBILIDADE;
  if (particulas < 0) {
    particulas = 0;
  }
  return particulas;
}




void medirParticulasDurante(unsigned long duracaoMs) {
  somaParticulas = 0.0;
  quantidadeAmostras = 0;




  unsigned long inicioMs = millis();
  while (millis() - inicioMs < duracaoMs) {
    // Liga o LED do sensor
    gpio_set_level(LED_POEIRA, 1);
    ets_delay_us(280);




    // Le o nivel de particulas
    float particula = lerParticulas();
    somaParticulas += particula;
    quantidadeAmostras++;




    // Desliga o LED
    gpio_set_level(LED_POEIRA, 0);




    Serial.print("Particulas: ");
    Serial.print(particula, 3);
    Serial.println(" mg/m3");




    delay(INTERVALO_AMOSTRA_MS);
  }




  if (quantidadeAmostras > 0) {
    particulasMedidas = somaParticulas / quantidadeAmostras;
  } else {
    particulasMedidas = 0.0;
  }
}




// ============================================================
// SEQUENCIA PRINCIPAL
// ============================================================
void executarSequencia() {
  Serial.println();
  Serial.println("========== INICIO DA SEQUENCIA ==========");




  // 0) Motor de passo sobe (subida inicial) e fica parado em cima
  Serial.println("0) Motor de passo subindo (posicao inicial)");
  girarPasso(SENTIDO_SOBE, TEMPO_PASSO_SOBE_INICIAL_MS);
  delay(TEMPO_ESPERA_CIMA_MS);




  // 1) Esteira andando - SENTIDO 1 (ate a camara escura) -> LED LIGADO
  Serial.println("1) Esteira ligada - sentido 1");
  ledLigado();
  esteiraSentido1();
  delay(TEMPO_ESTEIRA_1);
  esteiraDesligada();
  ledDesligado();
  Serial.println("   Esteira parada");




  // 2) Medicao de luminosidade ANTES
  Serial.println("2) Medindo luminosidade (ANTES)...");
  luxAntes = medirLuminosidade();
  Serial.print("   Luminosidade ANTES: ");
  Serial.print(luxAntes, 2);
  Serial.println(" lux");




  // 3) Esteira anda novamente - SENTIDO 1 (camara escura -> camara de desgaste)
  //    LED permanece DESLIGADO
  Serial.println("3) Esteira ligada - sentido 1 - posicionar tecido");
  esteiraSentido1();
  delay(TEMPO_ESTEIRA_2);
  esteiraDesligada();
  Serial.println("   Esteira parada");




  // 4) Motor de passo desce o motor de desgaste e para
  Serial.println("4) Motor de passo descendo");
  girarPasso(SENTIDO_DESCE, TEMPO_PASSO_DESCE_MS);




  // 5) Pausa
  registrar("NEMA PARADO");
  desgasteDesligado();
  delay(500);




  // 6) Lixa liga por TEMPO_DESGASTE (1,5 s) com o motor de passo parado
  Serial.println("6) LIGANDO LIXA AGORA");
  registrar("GPIO 25 = LOW");
  digitalWrite(RELE_DESGASTE, LOW);




  delay(TEMPO_DESGASTE);




  Serial.println("DESLIGANDO LIXA");
  registrar("GPIO 25 = HIGH");
  digitalWrite(RELE_DESGASTE, HIGH);




  Serial.print("   Media de particulas: ");
  Serial.print(particulasMedidas, 3);
  Serial.println(" mg/m3");




  // 7) Pausa
  registrar("Pausa de 0,5 s");
  delay(TEMPO_PAUSA_MS);




  // 8) Motor de passo sobe (sentido contrario) e para
  Serial.println("8) Motor de passo subindo");
  girarPasso(SENTIDO_SOBE, TEMPO_PASSO_SOBE_MS);




  // 9) Esteira anda no sentido contrario - SENTIDO 2
  //    (tecido volta a passar pela camara escura) -> LED LIGADO
  Serial.println("9) Esteira ligada - sentido 2");
  ledLigado();
  esteiraSentido2();
  delay(TEMPO_ESTEIRA_3);
  esteiraDesligada();
  ledDesligado();
  Serial.println("   Esteira parada");




  // 10) Sensor de luminosidade mede DEPOIS e armazena
  Serial.println("10) Medindo luminosidade (DEPOIS)...");
  luxDepois = medirLuminosidade();
  Serial.print("   Luminosidade DEPOIS: ");
  Serial.print(luxDepois, 2);
  Serial.println(" lux");




  // 11) Esteira anda mais - SENTIDO 2 (LED permanece DESLIGADO)
  Serial.println("11) Esteira ligada - sentido 2");
  esteiraSentido2();
  delay(TEMPO_ESTEIRA_3);
  esteiraDesligada();
  Serial.println("   Esteira parada");




  // 12) Apresentar resultados
  apresentarResultados();




  Serial.println("========== FIM DA SEQUENCIA ==========");
}




// ============================================================
// APRESENTACAO DOS RESULTADOS NO SERIAL
// ============================================================
void apresentarResultados() {
  Serial.println();
  Serial.println("---------- RESULTADOS FINAIS ----------");




  if (luxAntes >= 0.0 && luxDepois >= 0.0) {
    double diferenca = luxAntes - luxDepois;
    Serial.print("Luminosidade ANTES : ");
    Serial.print(luxAntes, 2);
    Serial.println(" lux");
    Serial.print("Luminosidade DEPOIS: ");
    Serial.print(luxDepois, 2);
    Serial.println(" lux");
    Serial.print("Diferenca de luminosidade (antes - depois): ");
    Serial.print(diferenca, 2);
    Serial.println(" lux");
  } else {
    Serial.println("Erro: nao foi possivel obter leituras de luminosidade validas.");
  }




  if (particulasMedidas >= 0.0) {
    Serial.print("Particulas medidas: ");
    Serial.print(particulasMedidas, 3);
    Serial.println(" mg/m3");
  } else {
    Serial.println("Erro: nao foi possivel obter leitura de particulas.");
  }




  Serial.println("--------------------------------------");
}




// ============================================================
// SETUP
// ============================================================
void setup() {
  // LIXA: define o nivel HIGH (desligada) no registrador ANTES de virar saida,
  // assim o pino ja nasce desligado, sem pulso
  gpio_set_level(GPIO_NUM_25, 1);
  gpio_set_direction(GPIO_NUM_25, GPIO_MODE_OUTPUT);
  pinMode(RELE_DESGASTE, OUTPUT);
  digitalWrite(RELE_DESGASTE, RELE_DESLIGADO);




  // ESTEIRA: dois reles (IN1/IN2), desligada = ambos LOW
  pinMode(ESTEIRA_IN1, OUTPUT);
  pinMode(ESTEIRA_IN2, OUTPUT);
  digitalWrite(ESTEIRA_IN1, LOW);
  digitalWrite(ESTEIRA_IN2, LOW);




  // LED DA CAMARA ESCURA: comeca desligado
  pinMode(LED, OUTPUT);
  ledDesligado();




  // MOTOR DE PASSO
  pinMode(PINO_PASSO, OUTPUT);
  pinMode(PINO_DIRECAO, OUTPUT);
  digitalWrite(PINO_PASSO, LOW);
  digitalWrite(PINO_DIRECAO, LOW);




  Serial.begin(115200);
  while (!Serial) {
    delay(50);
  }
  delay(1000);




  // ===== BANNER DE BOAS-VINDAS =====
  Serial.println();
  Serial.println("======================================");
  Serial.println("   SISTEMA DE TESTE DE DESGASTE EM");
  Serial.println("           TECIDO - SOFHMES");
  Serial.println("======================================");




  // ===== LOGIN =====
  Serial.println();
  Serial.println("== LOGIN ==");
  Serial.print("usuario: ");
  String usuario = lerEntrada();
  Serial.println();
  Serial.print("senha: ");
  String senha = lerEntrada();
  Serial.println();




  if (usuario != USUARIO_SISTEMA || senha != SENHA_SISTEMA) {
    Serial.println();
    Serial.println("Credenciais invalidas. Voltando ao login...");
    delay(2000);
    ESP.restart();
  }




  Serial.println("Login realizado com sucesso.");




  // Tudo desligado no inicio
  esteiraDesligada();
  desgasteDesligado();
  ledDesligado();
  digitalWrite(PINO_PASSO, LOW);




  // ===== INICIALIZACOES DOS SENSORES =====
  Wire.begin();
  iniciarLuminosidade();
  iniciarSensorParticulas();




  registrar("Sistema iniciado");
}




// ============================================================
// LOOP
// ============================================================
void loop() {
  while (true) {
    // Pergunta se o sistema deve iniciar
    Serial.println();
    Serial.print("Iniciar sistema? (SIM/NAO): ");
    String resposta = lerEntrada();
    resposta.toUpperCase();




    if (resposta != "SIM") {
      Serial.println("Sistema encerrado. Reiniciando...");
      delay(2000);
      ESP.restart();
    }




    // Roda toda a sequencia
    executarSequencia();




    // Depois de rodar, pergunta se deve encerrar
    Serial.println();
    Serial.print("Encerrar sistema? (SIM/NAO): ");
    String respostaEncerrar = lerEntrada();
    respostaEncerrar.toUpperCase();




    if (respostaEncerrar == "SIM") {
      Serial.println("Sistema encerrado. Reiniciando...");
      delay(2000);
      ESP.restart();
    }
    // Se NAO, volta ao inicio do loop
  }
}




// ============================================================
// UTILITARIO - LEITURA DE ENTRADA DO SERIAL
// ============================================================
String lerEntrada() {
  String linha = "";
  while (true) {
    while (Serial.available() == 0) {
      delay(10);
    }
    char caractere = Serial.read();




    if (caractere == '\n') // Enter conclui a entrada
    {
      break;
    }
    if (caractere == '\r') {
      continue; // ignora \r (para Windows)
    }




    linha += caractere;
    Serial.print(caractere); // ecoa o que foi digitado
  }
  linha.trim();
  return linha;
}




// ============================================================
// UTILITARIO - IMPRESSAO DE ERROS I2C
// ============================================================
void imprimirErro(byte erro) {
  Serial.print("Erro I2C: ");
  Serial.print(erro, DEC);
  Serial.print(", ");
  switch (erro) {
    case 0:
      Serial.println("sucesso");
      break;
    case 1:
      Serial.println("dados longos demais para o buffer de transmissao");
      break;
    case 2:
      Serial.println("NACK no endereco (desconectado?)");
      break;
    case 3:
      Serial.println("NACK nos dados");
      break;
    case 4:
      Serial.println("outro erro");
      break;
    default:
      Serial.println("erro desconhecido");
      break;
  }
}
