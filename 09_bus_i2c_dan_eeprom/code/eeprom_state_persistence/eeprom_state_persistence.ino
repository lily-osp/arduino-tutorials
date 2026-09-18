/**
 * eeprom_state_persistence.ino
 * Modul 09: Menyimpan Status Relay dan Counter Operasi ke EEPROM Non-Volatile Internal
 * 
 * Hardware:
 * - Arduino Uno R3
 * - Modul Relay 5V : Pin D8 (Active-LOW)
 * - LED Indikator  : Pin D7 via Resistor 220 Ohm ke GND
 * - Tombol TOGGLE  : Pin D2 ke GND (INPUT_PULLUP)
 * - Tombol RESET   : Pin D3 ke GND (INPUT_PULLUP)
 * - LCD 1602 I2C   : SDA -> A4, SCL -> A5, VCC -> 5V, GND -> GND
 * 
 * Konsep Penting EEPROM:
 * 1. Validasi Magic Number: Mencegah pembacaan data acak saat memori EEPROM masih perawan (default 0xFF).
 * 2. EEPROM.put() vs write(): put() memanfaatkan mekanisme update() internal yang hanya menulis
 *    ke sel silikon jika datanya benar-benar berubah, menghemat batas 100.000 siklus write endurance.
 * 3. State Recovery: Status sakelar relay dan angka total siklus operasi dipulihkan otomatis
 *    setelah Arduino dicabut atau mengalami mati lampu mendadak.
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const uint8_t PIN_RELAY      = 8;
const uint8_t PIN_BTN_TOGGLE = 2;
const uint8_t PIN_BTN_RESET  = 3;
const uint8_t PIN_LED        = 7;

const uint8_t RELAY_ON  = LOW;  // Active-LOW relay trigger
const uint8_t RELAY_OFF = HIGH; // Standby relay

// Struktur data yang dikemas rapi untuk disimpan ke EEPROM
struct DataSimpanan {
    uint16_t magicNumber;    // Pengenal validitas integritas data (2 byte)
    uint32_t counterOperasi; // Jumlah siklus tombol ditekan (4 byte)
    bool statusRelay;        // Status terakhir relay ON/OFF (1 byte)
};

const uint16_t KODE_VALID  = 0xABCD; // Kunci penanda integritas data
const int ALAMAT_EEPROM    = 0;      // Mulai penyimpanan dari alamat byte ke-0

DataSimpanan memori;

// Variabel debounce tombol
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
    Serial.println(F("\n=== SISTEM PERSISTENSI STATE EEPROM ==="));

    // 1. Baca struktur data dari EEPROM internal
    EEPROM.get(ALAMAT_EEPROM, memori);

    // 2. Cek apakah memori sudah pernah diformat dengan kunci valid
    if (memori.magicNumber != KODE_VALID) {
        Serial.println(F("[FORMAT] Memori baru terdeteksi. Menginisialisasi default..."));
        memori.magicNumber    = KODE_VALID;
        memori.counterOperasi = 0;
        memori.statusRelay    = false;
        simpanKeEeprom();
    } else {
        Serial.println(F("[RESTORE] Data valid ditemukan. Memulihkan status terakhir..."));
    }

    // 3. Terapkan status hardware sesuai hasil restorasi memori
    if (memori.statusRelay) {
        digitalWrite(PIN_RELAY, RELAY_ON);
        digitalWrite(PIN_LED, HIGH);
        Serial.println(F(" -> Status dipulihkan: Relay AKTIF."));
    } else {
        digitalWrite(PIN_RELAY, RELAY_OFF);
        digitalWrite(PIN_LED, LOW);
        Serial.println(F(" -> Status dipulihkan: Relay STANDBY."));
    }

    lcd.init();
    lcd.backlight();
    perbaruiTampilan();
}

void loop() {
    uint32_t sekarang = millis();

    // -------------------------------------------------------------------------
    // 1. Tombol Toggle Relay (D2)
    // -------------------------------------------------------------------------
    int bacaToggle = digitalRead(PIN_BTN_TOGGLE);
    if (bacaToggle != statusToggleTerakhir) {
        waktuToggle = sekarang;
    }
    if ((sekarang - waktuToggle) > JEDA_DEBOUNCE) {
        static int statusToggleStabil = HIGH;
        if (bacaToggle != statusToggleStabil) {
            statusToggleStabil = bacaToggle;
            if (statusToggleStabil == LOW) {
                // Balik status relay dan naikkan counter
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

    // -------------------------------------------------------------------------
    // 2. Tombol Reset Counter (D3)
    // -------------------------------------------------------------------------
    int bacaReset = digitalRead(PIN_BTN_RESET);
    if (bacaReset != statusResetTerakhir) {
        waktuReset = sekarang;
    }
    if ((sekarang - waktuReset) > JEDA_DEBOUNCE) {
        static int statusResetStabil = HIGH;
        if (bacaReset != statusResetStabil) {
            statusResetStabil = bacaReset;
            if (statusResetStabil == LOW) {
                memori.counterOperasi = 0;
                simpanKeEeprom();
                perbaruiTampilan();
                Serial.println(F("[RESET] Counter operasi dikembalikan ke 0."));
            }
        }
    }
    statusResetTerakhir = bacaReset;
}

void simpanKeEeprom() {
    // EEPROM.put menggunakan EEPROM.update di level per-byte,
    // hanya menulis ke silikon fisik jika nilainya berbeda.
    EEPROM.put(ALAMAT_EEPROM, memori);
    Serial.print(F("[SIMPAN] Counter: "));
    Serial.print(memori.counterOperasi);
    Serial.print(F(" | Status Relay: "));
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
