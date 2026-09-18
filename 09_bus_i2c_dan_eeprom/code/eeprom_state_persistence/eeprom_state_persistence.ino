// eeprom_state_persistence.ino
// Modul 09: Menyimpan status relay dan counter operasional ke EEPROM internal secara persisten

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const uint8_t PIN_RELAY      = 8;
const uint8_t PIN_BTN_TOGGLE = 2;
const uint8_t PIN_BTN_RESET  = 3;
const uint8_t PIN_LED        = 7;

const uint8_t RELAY_ON  = LOW;
const uint8_t RELAY_OFF = HIGH;

struct DataSimpanan {
  uint16_t magicNumber;
  uint32_t counterOperasi;
  bool statusRelay;
};

const uint16_t KODE_VALID = 0xABCD;
const int ALAMAT_EEPROM    = 0;

DataSimpanan memori;

int statusToggleTerakhir = HIGH;
int statusResetTerakhir  = HIGH;
uint32_t waktuToggle     = 0;
uint32_t waktuReset      = 0;
const uint32_t JEDA_DEBOUNCE = 50;

void perbaruiTampilan();
void simpanKeEeprom();

void setup() {
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN_TOGGLE, INPUT_PULLUP);
  pinMode(PIN_BTN_RESET, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println(F("\n--- Sistem State Persistence EEPROM ---"));

  EEPROM.get(ALAMAT_EEPROM, memori);

  if (memori.magicNumber != KODE_VALID) {
    Serial.println(F("Inisialisasi memori EEPROM baru..."));
    memori.magicNumber    = KODE_VALID;
    memori.counterOperasi = 0;
    memori.statusRelay    = false;
    simpanKeEeprom();
  } else {
    Serial.println(F("Data valid ditemukan di EEPROM."));
  }

  if (memori.statusRelay) {
    digitalWrite(PIN_RELAY, RELAY_ON);
    digitalWrite(PIN_LED, HIGH);
    Serial.println(F("Status dipulihkan: Relay AKTIF."));
  } else {
    digitalWrite(PIN_RELAY, RELAY_OFF);
    digitalWrite(PIN_LED, LOW);
    Serial.println(F("Status dipulihkan: Relay MATI."));
  }

  lcd.init();
  lcd.backlight();
  perbaruiTampilan();
}

void loop() {
  uint32_t sekarang = millis();

  int bacaToggle = digitalRead(PIN_BTN_TOGGLE);
  if (bacaToggle != statusToggleTerakhir) waktuToggle = sekarang;
  if ((sekarang - waktuToggle) > JEDA_DEBOUNCE) {
    static int statusToggleStabil = HIGH;
    if (bacaToggle != statusToggleStabil) {
      statusToggleStabil = bacaToggle;
      if (statusToggleStabil == LOW) {
        memori.statusRelay = !memori.statusRelay;
        memori.counterOperasi++;

        digitalWrite(PIN_RELAY, memori.statusRelay ? RELAY_ON : RELAY_OFF);
        digitalWrite(PIN_LED, memori.statusRelay ? HIGH : LOW);

        simpanKeEeprom();
        perbaruiTampilan();
      }
    }
  }
  statusToggleTerakhir = bacaToggle;

  int bacaReset = digitalRead(PIN_BTN_RESET);
  if (bacaReset != statusResetTerakhir) waktuReset = sekarang;
  if ((sekarang - waktuReset) > JEDA_DEBOUNCE) {
    static int statusResetStabil = HIGH;
    if (bacaReset != statusResetStabil) {
      statusResetStabil = bacaReset;
      if (statusResetStabil == LOW) {
        memori.counterOperasi = 0;
        simpanKeEeprom();
        perbaruiTampilan();
        Serial.println(F("Counter di-reset ke 0."));
      }
    }
  }
  statusResetTerakhir = bacaReset;
}

void simpanKeEeprom() {
  EEPROM.put(ALAMAT_EEPROM, memori);
  Serial.print(F("EEPROM Disimpan | Counter: "));
  Serial.print(memori.counterOperasi);
  Serial.print(F(" | Relay: "));
  Serial.println(memori.statusRelay ? F("ON") : F("OFF"));
}

void perbaruiTampilan() {
  lcd.setCursor(0, 0);
  char baris0[17];
  snprintf(baris0, sizeof(baris0), "Relay: %-4s Cnt:%-3lu", 
           memori.statusRelay ? "ON" : "OFF", 
           memori.counterOperasi);
  lcd.print(baris0);

  lcd.setCursor(0, 1);
  lcd.print(F("D2:Tgl  D3:Reset"));
}
