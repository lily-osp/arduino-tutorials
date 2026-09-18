// button_toggle.ino
// Modul 05: Menyalakan dan mematikan LED setiap kali tombol ditekan sekali (Toggle) dengan Software Debounce

const uint8_t PIN_LED    = 7;
const uint8_t PIN_BUTTON = 2;

bool statusLed = false;
int statusTombolTerakhir   = HIGH;
int statusTombolSekarang   = HIGH;
uint32_t waktuDebounceTerakhir = 0;
const uint32_t JEDA_DEBOUNCE   = 50;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  digitalWrite(PIN_LED, LOW);
}

void loop() {
  int pembacaan = digitalRead(PIN_BUTTON);

  if (pembacaan != statusTombolTerakhir) {
    waktuDebounceTerakhir = millis();
  }

  if ((millis() - waktuDebounceTerakhir) > JEDA_DEBOUNCE) {
    if (pembacaan != statusTombolSekarang) {
      statusTombolSekarang = pembacaan;

      if (statusTombolSekarang == LOW) {
        statusLed = !statusLed;
        digitalWrite(PIN_LED, statusLed ? HIGH : LOW);
      }
    }
  }

  statusTombolTerakhir = pembacaan;
}
