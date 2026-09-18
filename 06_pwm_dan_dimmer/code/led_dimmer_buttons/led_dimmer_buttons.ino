// led_dimmer_buttons.ino
// Modul 06: Mengatur tingkat kecerahan LED menggunakan 2 tombol (Up & Down) dengan PWM

const uint8_t PIN_LED         = 9;
const uint8_t PIN_BTN_UP      = 2;
const uint8_t PIN_BTN_DOWN    = 3;

int levelKecerahan = 0;
const int LANGKAH_KECERAHAN = 25;

int statusUpTerakhir     = HIGH;
int statusDownTerakhir   = HIGH;
uint32_t waktuUpTerakhir   = 0;
uint32_t waktuDownTerakhir = 0;
const uint32_t JEDA_DEBOUNCE = 50;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN_UP, INPUT_PULLUP);
  pinMode(PIN_BTN_DOWN, INPUT_PULLUP);

  analogWrite(PIN_LED, levelKecerahan);

  Serial.begin(115200);
  Serial.println(F("Dimmer LED Siap. Tekan D2 (Up) atau D3 (Down)."));
}

void loop() {
  uint32_t sekarang = millis();

  // Tombol UP (D2)
  int bacaUp = digitalRead(PIN_BTN_UP);
  if (bacaUp != statusUpTerakhir) {
    waktuUpTerakhir = sekarang;
  }
  if ((sekarang - waktuUpTerakhir) > JEDA_DEBOUNCE) {
    static int statusUpStabil = HIGH;
    if (bacaUp != statusUpStabil) {
      statusUpStabil = bacaUp;
      if (statusUpStabil == LOW) {
        levelKecerahan += LANGKAH_KECERAHAN;
        if (levelKecerahan > 255) levelKecerahan = 255;
        analogWrite(PIN_LED, levelKecerahan);
        laporKecerahan();
      }
    }
  }
  statusUpTerakhir = bacaUp;

  // Tombol DOWN (D3)
  int bacaDown = digitalRead(PIN_BTN_DOWN);
  if (bacaDown != statusDownTerakhir) {
    waktuDownTerakhir = sekarang;
  }
  if ((sekarang - waktuDownTerakhir) > JEDA_DEBOUNCE) {
    static int statusDownStabil = HIGH;
    if (bacaDown != statusDownStabil) {
      statusDownStabil = bacaDown;
      if (statusDownStabil == LOW) {
        levelKecerahan -= LANGKAH_KECERAHAN;
        if (levelKecerahan < 0) levelKecerahan = 0;
        analogWrite(PIN_LED, levelKecerahan);
        laporKecerahan();
      }
    }
  }
  statusDownTerakhir = bacaDown;
}

void laporKecerahan() {
  Serial.print(F("Tingkat Kecerahan PWM: "));
  Serial.print(levelKecerahan);
  Serial.print(F(" / 255 ("));
  Serial.print((levelKecerahan * 100) / 255);
  Serial.println(F("%)"));
}
