// lcd1602_interactive.ino
// Modul 07: Menampilkan counter interaktif pada I2C LCD 1602 dengan kontrol Tombol & Serial

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const uint8_t PIN_BTN_UP   = 2;
const uint8_t PIN_BTN_DOWN = 3;

int counter = 0;

int statusUpTerakhir     = HIGH;
int statusDownTerakhir   = HIGH;
uint32_t waktuUpTerakhir   = 0;
uint32_t waktuDownTerakhir = 0;
const uint32_t JEDA_DEBOUNCE = 50;

const uint8_t panahAtas[8] = {
  B00100, B01110, B10101, B00100, B00100, B00100, B00100, B00000
};

const uint8_t panahBawah[8] = {
  B00100, B00100, B00100, B00100, B10101, B01110, B00100, B00000
};

void perbaruiTampilanLcd();

void setup() {
  pinMode(PIN_BTN_UP, INPUT_PULLUP);
  pinMode(PIN_BTN_DOWN, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println(F("Sistem Antarmuka LCD Siap."));
  Serial.println(F("Ketik '+' untuk naik, '-' untuk turun, 'r' untuk reset."));

  lcd.init();
  lcd.backlight();

  lcd.createChar(0, (uint8_t*)panahAtas);
  lcd.createChar(1, (uint8_t*)panahBawah);

  perbaruiTampilanLcd();
}

void loop() {
  uint32_t sekarang = millis();
  bool perluUpdateLcd = false;

  int bacaUp = digitalRead(PIN_BTN_UP);
  if (bacaUp != statusUpTerakhir) waktuUpTerakhir = sekarang;
  if ((sekarang - waktuUpTerakhir) > JEDA_DEBOUNCE) {
    static int statusUpStabil = HIGH;
    if (bacaUp != statusUpStabil) {
      statusUpStabil = bacaUp;
      if (statusUpStabil == LOW) {
        counter++;
        perluUpdateLcd = true;
      }
    }
  }
  statusUpTerakhir = bacaUp;

  int bacaDown = digitalRead(PIN_BTN_DOWN);
  if (bacaDown != statusDownTerakhir) waktuDownTerakhir = sekarang;
  if ((sekarang - waktuDownTerakhir) > JEDA_DEBOUNCE) {
    static int statusDownStabil = HIGH;
    if (bacaDown != statusDownStabil) {
      statusDownStabil = bacaDown;
      if (statusDownStabil == LOW) {
        counter--;
        perluUpdateLcd = true;
      }
    }
  }
  statusDownTerakhir = bacaDown;

  while (Serial.available() > 0) {
    char karakter = (char)Serial.read();
    if (karakter == '+') {
      counter++;
      perluUpdateLcd = true;
    } else if (karakter == '-') {
      counter--;
      perluUpdateLcd = true;
    } else if (karakter == 'r' || karakter == 'R') {
      counter = 0;
      perluUpdateLcd = true;
    }
  }

  if (perluUpdateLcd) {
    perbaruiTampilanLcd();
  }
}

void perbaruiTampilanLcd() {
  lcd.setCursor(0, 0);
  lcd.print(F("PANEL KONTROL "));
  lcd.write(0);
  lcd.write(1);

  lcd.setCursor(0, 1);
  char buffer[17];
  snprintf(buffer, sizeof(buffer), "Nilai: %-6d   ", counter);
  lcd.print(buffer);

  Serial.print(F("Nilai Counter Terkini: "));
  Serial.println(counter);
}
