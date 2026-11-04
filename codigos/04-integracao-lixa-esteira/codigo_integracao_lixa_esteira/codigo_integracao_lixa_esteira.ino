#define STEP_PIN 32
#define DIR_PIN  33
#define RELE_PIN 25


// Lógica do relé: este módulo liga com LOW e desliga com HIGH
#define RELE_LIGADO    LOW
#define RELE_DESLIGADO HIGH


// Tempos do ciclo (ms)
#define TEMPO_MOTOR_MS 1500   // descida e subida
#define TEMPO_LIXA_MS  1500   // lixa ligada


// Gira o motor de passo por 2,5 segundos no sentido escolhido
void girarPor2_5Segundos(bool sentido) {


  digitalWrite(RELE_PIN, RELE_DESLIGADO);   // garante lixa desligada


  digitalWrite(DIR_PIN, sentido);
  delay(50);


  unsigned long inicio = millis();


  while (millis() - inicio < TEMPO_MOTOR_MS) {


    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(1500);


    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(1500);
  }


  digitalWrite(STEP_PIN, LOW);
}


// Liga a lixa por 3 segundos (motor de passo parado)
void lixarPor3Segundos() {


  digitalWrite(RELE_PIN, RELE_LIGADO);
  delay(TEMPO_LIXA_MS);
  digitalWrite(RELE_PIN, RELE_DESLIGADO);
}


void setup() {
  // Define o nível ANTES do pinMode, para o relé não disparar na partida
  digitalWrite(RELE_PIN, RELE_DESLIGADO);
  pinMode(RELE_PIN, OUTPUT);


  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);


  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);
}


void loop() {


  // 1. Motor de passo desce a lixa por 2,5 s
  girarPor2_5Segundos(HIGH);


  // 2. Tudo parado
  delay(500);


  // 3. Lixa liga por 3 s
  lixarPor3Segundos();


  // 4. Tudo parado
  delay(500);


  // 5. Motor de passo sobe a lixa por 2,5 s
  girarPor2_5Segundos(LOW);


  // 6. Pausa antes de repetir
  delay(2000);
}
