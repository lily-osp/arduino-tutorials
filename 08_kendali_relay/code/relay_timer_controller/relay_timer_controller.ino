// relay_timer_controller.ino
// Modul 08: Mengendalikan modul relay 5V (Active-LOW) dengan tombol toggle, auto-cutoff timer, dan status LCD

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const uint8_t PIN_RELAY  = 8;
const uint8_t PIN_BTN    = 2;
const uint8_t PIN_LED    = 7;

const uint8_t RELAY_ON  = LOW;
const uint8_t RELAY_OFF = HIGH;

bool statusRelayAktif = false;
uint32_t waktuMulaiRelay = 0;
const uint32_t DURASI_AUTO_OFF = 10000;

int statusTombolTerakhir = HIGH;
uint32_t waktuDebounce   = 0;
const uint32_t JEDA_DEBOUNCE = 50;

uint32_t waktuAksiRelayTerakhir = 0;
const uint32_t JEDA_ANTI_CHATTER = 500;

void perbaruiTampilanLcd(int sisaDetik);
void aktifkanRelay(uint32_t waktuMulai);
void matikanRelay();

void setup() {
  digitalWrite(PIN_RELAY, RELAY_OFF);
  pinMode(PIN_RELAY, OUTPUT);

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  pinMode(PIN_BTN, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println(F("Pengendali Relay Siap."));

  lcd.init();
  lcd.backlight();
  perbaruiTampilanLcd(0);
}

void loop() {
  uint32_t sekarang = millis();

  int bacaTombol = digitalRead(PIN_BTN);
  if (bacaTombol != statusTombolTerakhir) {
    waktuDebounce = sekarang;
  }
  if ((sekarang - waktuDebounce) > JEDA_DEBOUNCE) {
    static int statusStabil = HIGH;
    if (bacaTombol != statusStabil) {
      statusStabil = bacaTombol;
      if (statusStabil == LOW) {
        if (sekarang - waktuAksiRelayTerakhir >= JEDA_ANTI_CHATTER) {
          waktuAksiRelayTerakhir = sekarang;

          if (!statusRelayAktif) {
            aktifkanRelay(sekarang);
          } else {
            matikanRelay();
          }
        }
      }
    }
  }
  statusTombolTerakhir = bacaTombol;

  if (statusRelayAktif) {
    uint32_t waktuBerjalan = sekarang - waktuMulaiRelay;
    if (waktuBerjalan >= DURASI_AUTO_OFF) {
      Serial.println(F("Timeout tercapai: Relay dimatikan otomatis."));
      matikanRelay();
    } else {
      static uint32_t waktuUpdateLcd = 0;
      if (sekarang - waktuUpdateLcd >= 500) {
        waktuUpdateLcd = sekarang;
        int sisaDetik = (DURASI_AUTO_OFF - waktuBerjalan) / 1000 + 1;
        perbaruiTampilanLcd(sisaDetik);
      }
    }
  }
}

void aktifkanRelay(uint32_t waktuMulai) {
  statusRelayAktif = true;
  waktuMulaiRelay = waktuMulai;
  digitalWrite(PIN_RELAY, RELAY_ON);
  digitalWrite(PIN_LED, HIGH);

  Serial.println(F("Relay: AKTIF (Timer 10 detik berjalan)"));
  perbaruiTampilanLcd(10);
}

void matikanRelay() {
  statusRelayAktif = false;
  digitalWrite(PIN_RELAY, RELAY_OFF);
  digitalWrite(PIN_LED, LOW);

  Serial.println(F("Relay: MATI (Standby)"));
  perbaruiTampilanLcd(0);
}

void perbaruiTampilanLcd(int sisaDetik) {
  lcd.setCursor(0, 0);
  lcd.print(F("STATUS: "));
  if (statusRelayAktif) {
    lcd.print(F("[AKTIF] "));
  } else {
    lcd.print(F("[STANDBY]"));
  }

  lcd.setCursor(0, 1);
  char buffer[17];
  if (statusRelayAktif) {
    snprintf(buffer, sizeof(buffer), "Auto-off: %2d s   ", sisaDetik);
  } else {
    snprintf(buffer, sizeof(buffer), "Tekan D2 -> ON  ");
  }
  lcd.print(buffer);
}
