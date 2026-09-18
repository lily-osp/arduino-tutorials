/**
 * relay_timer_controller.ino
 * Modul 08: Pengendali Modul Relay 5V dengan Tombol Toggle, Auto-Cutoff, dan LCD
 * 
 * Hardware:
 * - Arduino Uno R3
 * - Modul Relay 5V 1-Channel:
 *     * VCC -> 5V Uno
 *     * GND -> GND Uno
 *     * IN  -> Pin D8 Uno (Active-LOW: 0V = ON, 5V = OFF)
 * - LED Indikator : Pin D7 via Resistor 220 Ohm ke GND
 * - Push Button   : Pin D2 ke GND (INPUT_PULLUP, Active-LOW)
 * - LCD 1602 I2C  : SDA -> A4, SCL -> A5, VCC -> 5V, GND -> GND
 * 
 * Fitur Keselamatan:
 * 1. Booting Glitch Prevention: Memanggil digitalWrite(HIGH) sebelum pinMode(OUTPUT)
 *    agar relay tidak menyala sesaat (glitch) saat mikrokontroler pertama kali menyala.
 * 2. Anti-Chatter Protection: Mencegah relay berpindah status lebih cepat dari 500ms
 *    untuk menjaga ketahanan fisik kontak tembaga relay.
 * 3. Auto-Cutoff Timer: Relay otomatis padam setelah 10 detik untuk mencegah beban overheat.
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const uint8_t PIN_RELAY  = 8;
const uint8_t PIN_BTN    = 2;
const uint8_t PIN_LED    = 7;

// Definisi logika Active-LOW modul relay
const uint8_t RELAY_ON  = LOW;  // Sinyal 0V memicu optocoupler menyalakan koil
const uint8_t RELAY_OFF = HIGH; // Sinyal 5V mematikan optocoupler (standby)

bool statusRelayAktif = false;
uint32_t waktuMulaiRelay = 0;
const uint32_t DURASI_AUTO_OFF = 10000; // Batas durasi kerja maksimal (10 detik)

// Variabel debounce tombol
int statusTombolTerakhir = HIGH;
uint32_t waktuDebounce   = 0;
const uint32_t JEDA_DEBOUNCE = 50;

// Proteksi mekanik anti-chatter
uint32_t waktuAksiRelayTerakhir = 0;
const uint32_t JEDA_ANTI_CHATTER = 500; // Jeda minimum antaraksi relay (500 milidetik)

void perbaruiTampilanLcd(int sisaDetik);
void aktifkanRelay(uint32_t waktuMulai);
void matikanRelay();

void setup() {
    // PENTING: Tulis logika HIGH ke output latch register SEBELUM mengeset mode OUTPUT.
    // Trik ini mencegah koil relay tersentak menyala (glitch) sesaat saat pin bertransisi dari Tri-state ke Output.
    digitalWrite(PIN_RELAY, RELAY_OFF);
    pinMode(PIN_RELAY, OUTPUT);

    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);

    pinMode(PIN_BTN, INPUT_PULLUP);

    Serial.begin(115200);
    Serial.println(F("\n=== PENGENDALI RELAY DENGAN PROTEKSI TIMER ==="));

    lcd.init();
    lcd.backlight();
    perbaruiTampilanLcd(0);
}

void loop() {
    uint32_t sekarang = millis();

    // -------------------------------------------------------------------------
    // 1. Baca Tombol Toggle (D2) dengan Debounce
    // -------------------------------------------------------------------------
    int bacaTombol = digitalRead(PIN_BTN);
    if (bacaTombol != statusTombolTerakhir) {
        waktuDebounce = sekarang;
    }
    if ((sekarang - waktuDebounce) > JEDA_DEBOUNCE) {
        static int statusStabil = HIGH;
        if (bacaTombol != statusStabil) {
            statusStabil = bacaTombol;
            if (statusStabil == LOW) {
                // 2. Proteksi Anti-Chatter: Pastikan sudah lewat 500ms sejak peralihan terakhir
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

    // -------------------------------------------------------------------------
    // 3. Auto-Cutoff Scheduler (Non-Blocking Timer)
    // -------------------------------------------------------------------------
    if (statusRelayAktif) {
        uint32_t waktuBerjalan = sekarang - waktuMulaiRelay;

        // Cek apakah waktu operasional sudah mencapai 10 detik
        if (waktuBerjalan >= DURASI_AUTO_OFF) {
            Serial.println(F("[AUTO-CUTOFF] Waktu habis: Relay dimatikan otomatis."));
            matikanRelay();
        } else {
            // Perbarui sisa waktu di layar LCD setiap 500ms agar efisien
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

    Serial.println(F("[AKSI] Relay: AKTIF (Timer 10 detik dimulai)"));
    perbaruiTampilanLcd(10);
}

void matikanRelay() {
    statusRelayAktif = false;
    digitalWrite(PIN_RELAY, RELAY_OFF);
    digitalWrite(PIN_LED, LOW);

    Serial.println(F("[AKSI] Relay: STANDBY (Mati)"));
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
