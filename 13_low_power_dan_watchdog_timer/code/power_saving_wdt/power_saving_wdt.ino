/**
 * power_saving_wdt.ino
 * Modul 13: Manajemen Daya (Sleep Mode) & Keandalan Sistem dengan Watchdog Timer (WDT)
 * Hardware: Arduino Uno R3, Push Button D2 (INT0), LED D13, Relay Pin D7
 */

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

const uint8_t PIN_WAKEUP_BUTTON = 2; // INT0
const uint8_t PIN_RELAY         = 7;

volatile bool terbangunOlehTombol = false;
uint32_t waktuAktivitasTerakhir = 0;
const uint32_t TIMEOUT_TIDUR_MS = 10000; // Masuk sleep mode jika idle 10 detik

// ISR Tombol INT0: Cukup tandai flag terbangun
void isrTombolBangun() {
    terbangunOlehTombol = true;
}

void masukModeTidurDalam() {
    Serial.println(F("\n[POWER] Sistem tidak aktif selama 10 detik."));
    Serial.println(F("[POWER] Memasuki SLEEP_MODE_PWR_DOWN..."));
    Serial.flush(); // Pastikan seluruh teks serial terkirim sebelum clock mati

    // Matikan beban eksternal sebelum tidur
    digitalWrite(PIN_RELAY, HIGH); // Standby relay
    digitalWrite(13, LOW);         // Matikan LED

    // 1. Matikan sementara Watchdog Timer sebelum tidur agar tidak mereset board saat lelap
    wdt_disable();

    // 2. Tentukan level mode tidur terdalam
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();

    // 3. Pasang interrupt agar dibangunkan saat pin D2 bernilai LOW (tombol ditekan)
    attachInterrupt(digitalPinToInterrupt(PIN_WAKEUP_BUTTON), isrTombolBangun, LOW);

    // 4. Masuk ke mode tidur (eksekusi CPU berhenti total di baris ini)
    sleep_cpu();

    // =========================================================================
    // KODE DI BAWAH INI AKAN DIEKSEKUSI SETELAH ARDUINO TERBANGUN KEMBALI
    // =========================================================================
    sleep_disable();
    detachInterrupt(digitalPinToInterrupt(PIN_WAKEUP_BUTTON));

    // Aktifkan kembali Watchdog Timer (timeout 2 detik)
    wdt_enable(WDTO_2S);

    waktuAktivitasTerakhir = millis();
    Serial.println(F("[POWER] Terjaga! CPU aktif kembali via interrupt tombol."));
}

void setup() {
    Serial.begin(115200);

    pinMode(13, OUTPUT);
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, HIGH);

    pinMode(PIN_WAKEUP_BUTTON, INPUT_PULLUP);

    // Aktifkan Hardware Watchdog Timer dengan durasi timeout 2 Detik
    wdt_enable(WDTO_2S);

    Serial.println(F("\n=== SISTEM SIAP: LOW-POWER & WATCHDOG TIMER ==="));
    Serial.println(F("- WDT Timeout: 2.0 Detik"));
    Serial.println(F("- Sleep Timeout: 10.0 Detik tanpa aktivitas"));
    Serial.println(F("- Tekan Tombol D2 untuk memberi tanda atau membangunkan sistem\n"));

    waktuAktivitasTerakhir = millis();
}

void loop() {
    // 1. Beri makan anjing penjaga (Kick/Reset Watchdog Timer secara berkala)
    wdt_reset();

    // 2. Cek apakah tombol ditekan selama sistem terjaga
    if (terbangunOlehTombol || digitalRead(PIN_WAKEUP_BUTTON) == LOW) {
        terbangunOlehTombol = false;
        waktuAktivitasTerakhir = millis();

        Serial.println(F("[AKSI] Tombol ditekan! Mengaktifkan Relay selama 2 detik..."));
        digitalWrite(PIN_RELAY, LOW); // Aktifkan relay
        digitalWrite(13, HIGH);

        // Jangan gunakan delay() blocking panjang yang melebihi timeout WDT (2s)
        // Kita gunakan loop non-blocking sambil tetap memanggil wdt_reset()
        uint32_t tMulai = millis();
        while (millis() - tMulai < 2000) {
            wdt_reset(); // Reset WDT agar tidak panik saat menunggu 2 detik
        }

        digitalWrite(PIN_RELAY, HIGH); // Matikan relay
        digitalWrite(13, LOW);
        Serial.println(F("[AKSI] Relay kembali ke posisi standby."));
    }

    // 3. Cek apakah sudah melebihi batas waktu idle untuk tidur
    if (millis() - waktuAktivitasTerakhir >= TIMEOUT_TIDUR_MS) {
        masukModeTidurDalam();
    }

    delay(20); // Sedikit jeda polling siklus loop
}
