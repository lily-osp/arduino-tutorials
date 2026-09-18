// smart_industrial_controller.ino
// Modul 10 Capstone: Pengendali Industri Berbasis FSM, Non-Blocking, dan Interrupt D2

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const uint8_t PIN_ESTOP_INT = 2;  // Hardware Interrupt INT0
const uint8_t PIN_BTN_START = 3;  // Tombol Mulai/Jeda
const uint8_t PIN_BTN_RESET = 4;  // Tombol Reset Alarm
const uint8_t PIN_LED       = 7;  // LED Status Multifungsi
const uint8_t PIN_RELAY     = 8;  // Modul Relay (Active-LOW)

const uint8_t RELAY_ON  = LOW;
const uint8_t RELAY_OFF = HIGH;

enum class StatusFsm : uint8_t {
  STANDBY,
  RUNNING,
  EMERGENCY_STOP
};

StatusFsm statusSistem = StatusFsm::STANDBY;

volatile bool flagKedaruratan = false;

uint32_t waktuMulaiRunning   = 0;
const uint32_t DURASI_SIKLUS = 10000;

struct LogIndustri {
  uint16_t magicNumber;
  uint32_t totalSiklusSukses;
  uint32_t totalInsidenEstop;
};

const uint16_t KODE_VALID = 0xF5A1;
const int ALAMAT_EEPROM    = 0;
LogIndustri catatan;

uint32_t timerLed = 0;
uint32_t timerLcd = 0;
bool statusFisikLed = false;

int statusStartTerakhir = HIGH;
int statusResetTerakhir = HIGH;
uint32_t debounceStart  = 0;
uint32_t debounceReset  = 0;
const uint32_t JEDA_DEBOUNCE = 50;

void isrEmergencyStop();
void tanganiTransisiFsm(uint32_t sekarang);
void tanganiTombolOperasi(uint32_t sekarang);
void perbaruiLcdDashboard();
void kelolaIndikatorLed(uint32_t sekarang);
void simpanLogEeprom();

void setup() {
  digitalWrite(PIN_RELAY, RELAY_OFF);
  pinMode(PIN_RELAY, OUTPUT);

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  pinMode(PIN_ESTOP_INT, INPUT_PULLUP);
  pinMode(PIN_BTN_START, INPUT_PULLUP);
  pinMode(PIN_BTN_RESET, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println(F("\n======================================"));
  Serial.println(F("  SMART INDUSTRIAL CONTROLLER - CAPSTONE"));
  Serial.println(F("======================================"));

  EEPROM.get(ALAMAT_EEPROM, catatan);
  if (catatan.magicNumber != KODE_VALID) {
    catatan.magicNumber       = KODE_VALID;
    catatan.totalSiklusSukses = 0;
    catatan.totalInsidenEstop = 0;
    simpanLogEeprom();
  }

  attachInterrupt(digitalPinToInterrupt(PIN_ESTOP_INT), isrEmergencyStop, FALLING);

  lcd.init();
  lcd.backlight();
  perbaruiLcdDashboard();
}

void isrEmergencyStop() {
  digitalWrite(PIN_RELAY, RELAY_OFF);
  flagKedaruratan = true;
}

void loop() {
  uint32_t sekarang = millis();

  if (flagKedaruratan) {
    flagKedaruratan = false;
    if (statusSistem != StatusFsm::EMERGENCY_STOP) {
      statusSistem = StatusFsm::EMERGENCY_STOP;
      catatan.totalInsidenEstop++;
      simpanLogEeprom();
      Serial.println(F("!!! ALARM: HARDWARE E-STOP TERPICU !!!"));
      perbaruiLcdDashboard();
    }
  }

  tanganiTombolOperasi(sekarang);
  tanganiTransisiFsm(sekarang);
  kelolaIndikatorLed(sekarang);

  if (sekarang - timerLcd >= 250) {
    timerLcd = sekarang;
    perbaruiLcdDashboard();
  }
}

void tanganiTransisiFsm(uint32_t sekarang) {
  switch (statusSistem) {
    case StatusFsm::STANDBY:
      digitalWrite(PIN_RELAY, RELAY_OFF);
      break;

    case StatusFsm::RUNNING:
      digitalWrite(PIN_RELAY, RELAY_ON);
      if (sekarang - waktuMulaiRunning >= DURASI_SIKLUS) {
        statusSistem = StatusFsm::STANDBY;
        digitalWrite(PIN_RELAY, RELAY_OFF);
        catatan.totalSiklusSukses++;
        simpanLogEeprom();
        Serial.println(F("Siklus kerja selesai secara normal. Kembali ke STANDBY."));
        perbaruiLcdDashboard();
      }
      break;

    case StatusFsm::EMERGENCY_STOP:
      digitalWrite(PIN_RELAY, RELAY_OFF);
      break;
  }
}

void tanganiTombolOperasi(uint32_t sekarang) {
  int bacaStart = digitalRead(PIN_BTN_START);
  if (bacaStart != statusStartTerakhir) debounceStart = sekarang;
  if ((sekarang - debounceStart) > JEDA_DEBOUNCE) {
    static int statusStartStabil = HIGH;
    if (bacaStart != statusStartStabil) {
      statusStartStabil = bacaStart;
      if (statusStartStabil == LOW) {
        if (statusSistem == StatusFsm::STANDBY) {
          statusSistem = StatusFsm::RUNNING;
          waktuMulaiRunning = sekarang;
          Serial.println(F("Sistem: Mulai RUNNING (Siklus 10 detik)."));
        } else if (statusSistem == StatusFsm::RUNNING) {
          statusSistem = StatusFsm::STANDBY;
          Serial.println(F("Sistem: Dijeda secara manual oleh operator."));
        }
        perbaruiLcdDashboard();
      }
    }
  }
  statusStartTerakhir = bacaStart;

  int bacaReset = digitalRead(PIN_BTN_RESET);
  if (bacaReset != statusResetTerakhir) debounceReset = sekarang;
  if ((sekarang - debounceReset) > JEDA_DEBOUNCE) {
    static int statusResetStabil = HIGH;
    if (bacaReset != statusResetStabil) {
      statusResetStabil = bacaReset;
      if (statusResetStabil == LOW) {
        if (statusSistem == StatusFsm::EMERGENCY_STOP) {
          if (digitalRead(PIN_ESTOP_INT) == HIGH) {
            statusSistem = StatusFsm::STANDBY;
            Serial.println(F("Alarm di-reset. Sistem kembali ke STANDBY siap operasi."));
            perbaruiLcdDashboard();
          } else {
            Serial.println(F("Gagal Reset: Tombol fisik E-STOP masih tertahan!"));
          }
        }
      }
    }
  }
  statusResetTerakhir = bacaReset;
}

void kelolaIndikatorLed(uint32_t sekarang) {
  uint32_t intervalKedip = 500;

  if (statusSistem == StatusFsm::RUNNING) {
    digitalWrite(PIN_LED, HIGH);
    return;
  } else if (statusSistem == StatusFsm::EMERGENCY_STOP) {
    intervalKedip = 75;
  }

  if (sekarang - timerLed >= intervalKedip) {
    timerLed = sekarang;
    statusFisikLed = !statusFisikLed;
    digitalWrite(PIN_LED, statusFisikLed ? HIGH : LOW);
  }
}

void perbaruiLcdDashboard() {
  char baris0[17];
  char baris1[17];

  switch (statusSistem) {
    case StatusFsm::STANDBY:
      snprintf(baris0, sizeof(baris0), "STATUS: STANDBY ");
      snprintf(baris1, sizeof(baris1), "D3:START Suk:%-3lu", catatan.totalSiklusSukses);
      break;

    case StatusFsm::RUNNING: {
      uint32_t waktuJalan = millis() - waktuMulaiRunning;
      int sisaDetik = (waktuJalan < DURASI_SIKLUS) ? (DURASI_SIKLUS - waktuJalan) / 1000 + 1 : 0;
      snprintf(baris0, sizeof(baris0), "STATUS: RUNNING ");
      snprintf(baris1, sizeof(baris1), "Sisa Waktu: %2ds  ", sisaDetik);
      break;
    }

    case StatusFsm::EMERGENCY_STOP:
      snprintf(baris0, sizeof(baris0), "!! E-STOP TRIP !");
      snprintf(baris1, sizeof(baris1), "D4:RESET Alm:%-3lu", catatan.totalInsidenEstop);
      break;
  }

  lcd.setCursor(0, 0);
  lcd.print(baris0);
  lcd.setCursor(0, 1);
  lcd.print(baris1);
}

void simpanLogEeprom() {
  EEPROM.put(ALAMAT_EEPROM, catatan);
}
