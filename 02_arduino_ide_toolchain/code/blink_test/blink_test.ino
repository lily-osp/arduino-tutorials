// blink_test.ino
// Modul 02: Uji Coba Pertama Mengedipkan LED Built-in Pin 13

const uint8_t LED_PIN = 13;

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}
