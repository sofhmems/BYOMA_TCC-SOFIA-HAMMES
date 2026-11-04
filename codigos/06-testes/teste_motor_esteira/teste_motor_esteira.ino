#define RELE_PIN 25


void setup() {
  pinMode(RELE_PIN, OUTPUT);


  // Motor começa desligado
  digitalWrite(RELE_PIN, LOW);
}


void loop() {


  // Liga o motor
  digitalWrite(RELE_PIN, HIGH);


  delay(5000); // gira por 5 segundos


  // Desliga o motor
  digitalWrite(RELE_PIN, LOW);


  delay(1000); // fica parado por 1 segundo


}
