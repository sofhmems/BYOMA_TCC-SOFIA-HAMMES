#define STEP_PIN 32
#define DIR_PIN 33


void girarPor5Segundos(bool sentido) {


  digitalWrite(DIR_PIN, sentido);


  unsigned long inicio = millis();


  while (millis() - inicio < 5000) {


    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(3000);


    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(3000);
  }
}


void setup() {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
}


void loop() {


  // gira 5 s para um lado
  girarPor5Segundos(HIGH);


  delay(2000);


  // gira 5 s para o outro
  girarPor5Segundos(LOW);


  delay(2000);
}
