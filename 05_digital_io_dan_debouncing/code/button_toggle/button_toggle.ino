/**
 * button_toggle.ino
 * Modul 05: Menyalakan dan Mematikan LED (Toggle) dengan Software Debouncing
 * 
 * Hardware:
 * - Arduino Uno R3
 * - LED 5mm: Anoda (+) terhubung ke pin D7 via Resistor 220 Ohm, Katoda (-) ke GND
 * - Push Button: Kaki 1 ke pin D2, Kaki 2 ke GND (tanpa resistor eksternal)
 * 
 * Konsep:
 * 1. INPUT_PULLUP mengaktifkan resistor internal 20k-50k Ohm ke 5V. Logika pin bernilai
 *    HIGH saat tombol terbuka, dan bernilai LOW saat tombol ditekan (Active-LOW).
 * 2. Software Debounce menyaring getaran pelat mekanis sakelar yang memantul selama
 *    5-20 ms agar penekanan tombol tidak terhitung berkali-kali.
 */

#include <Arduino.h>

const uint8_t PIN_LED    = 7;
const uint8_t PIN_BUTTON = 2;

// Variabel status LED
bool statusLed = false;

// Variabel pelacak status tombol untuk debounce
int statusTombolTerakhir   = HIGH; // Bacaan instan pada siklus loop sebelumnya
int statusTombolSekarang   = HIGH; // Bacaan yang sudah terbukti stabil
uint32_t waktuDebounceTerakhir = 0;
const uint32_t JEDA_DEBOUNCE   = 50; // Ambang batas kestabilan sinyal (50 milidetik)

void setup() {
    pinMode(PIN_LED, OUTPUT);
    // Aktifkan pull-up internal untuk menghindari pin mengambang (floating state)
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    
    // Pastikan LED mati saat board pertama kali booting
    digitalWrite(PIN_LED, LOW);
}

void loop() {
    // 1. Baca sinyal instan saat ini
    int pembacaan = digitalRead(PIN_BUTTON);

    // 2. Jika ada perubahan logika (akibat getaran kontak atau penekanan awal), catat stempel waktu
    if (pembacaan != statusTombolTerakhir) {
        waktuDebounceTerakhir = millis();
    }

    // 3. Jika sinyal bertahan stabil lebih lama dari ambang batas debounce (50ms)
    if ((millis() - waktuDebounceTerakhir) > JEDA_DEBOUNCE) {
        // Jika logika yang stabil ini berbeda dari status tombol sebelumnya
        if (pembacaan != statusTombolSekarang) {
            statusTombolSekarang = pembacaan;

            // Trigger aksi hanya pada transisi jatuh (Falling Edge: tombol baru saja ditekan)
            if (statusTombolSekarang == LOW) {
                statusLed = !statusLed; // Balik status logika LED
                digitalWrite(PIN_LED, statusLed ? HIGH : LOW);
            }
        }
    }

    // 4. Simpan hasil bacaan instan untuk perbandingan pada siklus loop berikutnya
    statusTombolTerakhir = pembacaan;
}
