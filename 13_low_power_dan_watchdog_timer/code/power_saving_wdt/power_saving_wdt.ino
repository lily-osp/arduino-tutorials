/**
 * power_saving_wdt.ino
 * Modul 13: Manajemen Daya (Sleep Mode) & Keandalan Sistem dengan Watchdog Timer (WDT)
 * 
 * Hardware:
 * - Arduino Uno R3
 * - Push Button (Wakeup) : Pin D2 ke GND (INT0 External Interrupt, Active-LOW)
 * - Modul Relay 5V       : Pin D7 (Active-LOW: LOW = ON, HIGH = Standby)
 * - LED Status           : Built-in LED Pin D13 (PB5)
 * 
 * Arsitektur Keandalan & Efisiensi:
 * 1. Deep Sleep: Jika tidak ada interaksi selama 10 detik, chip memasuki SLEEP_MODE_PWR_DOWN.
 *    Seluruh osilator dimatikan, konsumsi arus CPU turun drastis ke level mikroampere.
 * 2. Wake-on-Interrupt: Menekan tombol pada Pin D2 memicu sinyal LOW yang seketika membangunkan CPU.
 * 3. Watchdog Timer (WDT): Penjaga perangkat keras independen dengan timeout 2.0 detik.
 *    Jika program terjebak infinite loop atau hang akibat noise EMI relay, WDT otomatis me-reboot board.
 */

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

const uint8_t PIN_WAKEUP_BUTTON = 2; // Pin interrupt eksternal INT0
const uint8_t PIN_RELAY         = 7;

volatile bool terbangunOlehTombol = false;
uint32_t waktuAktivitasTerakhir = 0;
const uint32_t TIMEOUT_TIDUR_MS = 10000; // Masuk sleep mode jika idle selama 10 detik

/**
 * ISR Tombol Bangun (INT0):
 * Hanya menandai flag bahwa sistem dibangunkan oleh interaksi tombol fisik.
 */
void isrTombolBangun() {
    terbangunOlehTombol = true;
}

void masukModeTidurDalam() {
    Serial.println(F("\n[POWER] Sistem tidak aktif selama 10 detik."));
    Serial.println(F("[POWER] Memasuki SLEEP_MODE_PWR_DOWN..."));
    
    // PENTING: Serial.flush() menunggu seluruh karakter di buffer hardware UART
    // selesai dikirim sebelum clock osilator CPU dimatikan total.
    Serial.flush();

    // Pastikan seluruh beban listrik mati sebelum tidur
    digitalWrite(PIN_RELAY, HIGH); // Standby relay
    digitalWrite(13, LOW);         // Matikan LED

    // 1. Matikan sementara Watchdog Timer sebelum tidur.
    // Jika tidak dimatikan, WDT akan memicu hardware reset setelah 2 detik saat CPU terlelap!
    wdt_disable();

    // 2. Konfigurasi mode tidur terdalam (Power-down Mode)
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();

    // 3. Pasang interrupt agar bangun saat pin D2 bernilai LOW (tombol ditekan)
    attachInterrupt(digitalPinToInterrupt(PIN_WAKEUP_BUTTON), isrTombolBangun, LOW);

    // 4. Masuk ke mode tidur: Eksekusi program CPU berhenti total pada baris ini
    sleep_cpu();

    // =========================================================================
    // BARIS DI BAWAH INI DIEKSEKUSI SETELAH ARDUINO TERBANGUN KEMBALI OLEH TOMBOL
    // =========================================================================
    sleep_disable();
    detachInterrupt(digitalPinToInterrupt(PIN_WAKEUP_BUTTON));

    // 5. Aktifkan kembali Watchdog Timer (timeout 2 detik) untuk menjaga program aktif
    wdt_enable(WDTO_2S);

    waktuAktivitasTerakhir = millis();
    Serial.println(F("[POWER] Terjaga! CPU aktif kembali via interrupt tombol."));
}

void setup() {
    Serial.begin(115200);

    pinMode(13, OUTPUT);
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, HIGH); // Standby relay

    pinMode(PIN_WAKEUP_BUTTON, INPUT_PULLUP);

    // Aktifkan Hardware Watchdog Timer dengan durasi timeout 2.0 Detik
    wdt_enable(WDTO_2S);

    Serial.println(F("\n=============================================="));
    Serial.println(F("   SISTEM SIAP: LOW-POWER & WATCHDOG TIMER    "));
    Serial.println(F("=============================================="));
    Serial.println(F("- WDT Timeout   : 2.0 Detik"));
    Serial.println(F("- Sleep Timeout : 10.0 Detik tanpa aktivitas"));
    Serial.println(F("- Tekan Tombol D2 untuk interaksi atau membangunkan CPU\n"));

    waktuAktivitasTerakhir = millis();
}

void loop() {
    // 1. Beri makan Watchdog Timer (Kick/Reset WDT) secara rutin di setiap siklus loop
    wdt_reset();

    // 2. Cek apakah ada interaksi tombol saat sistem aktif
    if (terbangunOlehTombol || digitalRead(PIN_WAKEUP_BUTTON) == LOW) {
        terbangunOlehTombol = false;
        waktuAktivitasTerakhir = millis();

        Serial.println(F("[AKSI] Tombol ditekan! Mengaktifkan Relay selama 2 detik..."));
        digitalWrite(PIN_RELAY, LOW); // Aktifkan relay
        digitalWrite(13, HIGH);

        // Jangan gunakan delay(2000) blocking yang dapat melampaui timeout WDT 2 detik!
        // Kita gunakan polling loop non-blocking sambil tetap memanggil wdt_reset()
        uint32_t tMulai = millis();
        while (millis() - tMulai < 2000) {
            wdt_reset(); // Reset WDT agar tidak memicu reboot saat menunggu
        }

        digitalWrite(PIN_RELAY, HIGH); // Matikan relay
        digitalWrite(13, LOW);
        Serial.println(F("[AKSI] Relay kembali ke posisi standby."));
    }

    // 3. Masuk ke mode tidur jika batas waktu idle 10 detik terlewati
    if (millis() - waktuAktivitasTerakhir >= TIMEOUT_TIDUR_MS) {
        masukModeTidurDalam();
    }

    delay(20); // Jeda singkat polling loop
}
