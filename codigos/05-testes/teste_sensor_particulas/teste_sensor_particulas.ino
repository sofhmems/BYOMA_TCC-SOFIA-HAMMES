#define RELE_PIN 26


void setup() {
  pinMode(RELE_PIN, OUTPUT);


  // Motor desligado no início
  digitalWrite(RELE_PIN, LOW);
}


void loop() {


  // Liga o motor
  digitalWrite(RELE_PIN, HIGH);


  delay(5000); // motor funcionando por 5 segundos


  // Desliga o motor
  digitalWrite(RELE_PIN, LOW);


  delay(5000); // motor parado por 5 segundos
}
