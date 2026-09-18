/**
 * led_dimmer_buttons.ino
 * Modul 06: Mengatur Tingkat Kecerahan LED Menggunakan 2 Tombol dengan Sinyal PWM
 * 
 * Hardware:
 * - Arduino Uno R3
 * - LED 5mm: Anoda (+) terhubung ke pin PWM D9 via Resistor 220 Ohm, Katoda (-) ke GND
 * - Tombol UP (Tambah Terang): Kaki 1 ke pin D2, Kaki 2 ke GND (Active-LOW)
 * - Tombol DOWN (Redupkan): Kaki 1 ke pin D3, Kaki 2 ke GND (Active-LOW)
 * 
 * Konsep PWM:
 * Pin D9 dikendalikan oleh Timer 1 dengan frekuensi pulsa 490 Hz. Nilai analogWrite
 * berkisar dari 0 (duty cycle 0% / mati total) hingga 255 (duty cycle 100% / terang maksimal).
 */

#include <Arduino.h>

const uint8_t PIN_LED         = 9; // Pin digital berkemampuan hardware PWM (Timer 1)
const uint8_t PIN_BTN_UP      = 2; // Tombol untuk menaikkan duty cycle
const uint8_t PIN_BTN_DOWN    = 3; // Tombol untuk menurunkan duty cycle

int levelKecerahan = 0;             // Nilai PWM aktif (rentang 0 - 255)
const int LANGKAH_KECERAHAN = 25;   // Nilai kenaikan/penurunan setiap penekanan (~10%)

// Variabel debounce untuk Tombol UP
int statusUpTerakhir       = HIGH;
uint32_t waktuUpTerakhir   = 0;

// Variabel debounce untuk Tombol DOWN
int statusDownTerakhir     = HIGH;
uint32_t waktuDownTerakhir = 0;

const uint32_t JEDA_DEBOUNCE = 50; // Waktu filter contact bounce sakelar (50ms)

void laporKecerahan() {
    Serial.print(F("Tingkat Kecerahan PWM: "));
    Serial.print(levelKecerahan);
    Serial.print(F(" / 255 (Duty Cycle: "));
    Serial.print((levelKecerahan * 100) / 255);
    Serial.println(F("%)"));
}

void setup() {
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_BTN_UP, INPUT_PULLUP);
    pinMode(PIN_BTN_DOWN, INPUT_PULLUP);

    // Set nilai awal duty cycle PWM ke 0 (LED mati)
    analogWrite(PIN_LED, levelKecerahan);

    Serial.begin(115200);
    Serial.println(F("\n=== SISTEM LED DIMMER PWM AKTIF ==="));
    Serial.println(F("- Tekan Tombol D2 untuk menaikkan kecerahan"));
    Serial.println(F("- Tekan Tombol D3 untuk meredupkan"));
}

void loop() {
    uint32_t sekarang = millis();

    // -------------------------------------------------------------------------
    // 1. Pengecekan Tombol UP (D2) dengan Software Debounce
    // -------------------------------------------------------------------------
    int bacaUp = digitalRead(PIN_BTN_UP);
    if (bacaUp != statusUpTerakhir) {
        waktuUpTerakhir = sekarang;
    }
    if ((sekarang - waktuUpTerakhir) > JEDA_DEBOUNCE) {
        static int statusUpStabil = HIGH;
        if (bacaUp != statusUpStabil) {
            statusUpStabil = bacaUp;
            // Transisi jatuh (tombol ditekan)
            if (statusUpStabil == LOW) {
                levelKecerahan += LANGKAH_KECERAHAN;
                // Batasi nilai atas agar tidak meluap dari batas 8-bit (255)
                if (levelKecerahan > 255) {
                    levelKecerahan = 255;
                }
                analogWrite(PIN_LED, levelKecerahan);
                laporKecerahan();
            }
        }
    }
    statusUpTerakhir = bacaUp;

    // -------------------------------------------------------------------------
    // 2. Pengecekan Tombol DOWN (D3) dengan Software Debounce
    // -------------------------------------------------------------------------
    int bacaDown = digitalRead(PIN_BTN_DOWN);
    if (bacaDown != statusDownTerakhir) {
        waktuDownTerakhir = sekarang;
    }
    if ((sekarang - waktuDownTerakhir) > JEDA_DEBOUNCE) {
        static int statusDownStabil = HIGH;
        if (bacaDown != statusDownStabil) {
            statusDownStabil = bacaDown;
            // Transisi jatuh (tombol ditekan)
            if (statusDownStabil == LOW) {
                levelKecerahan -= LANGKAH_KECERAHAN;
                // Batasi nilai bawah agar tidak menjadi angka negatif
                if (levelKecerahan < 0) {
                    levelKecerahan = 0;
                }
                analogWrite(PIN_LED, levelKecerahan);
                laporKecerahan();
            }
        }
    }
    statusDownTerakhir = bacaDown;
}
