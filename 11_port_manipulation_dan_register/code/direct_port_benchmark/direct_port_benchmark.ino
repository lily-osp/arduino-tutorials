/**
 * direct_port_benchmark.ino
 * Modul 11: Benchmark Kecepatan Direct Port Manipulation vs digitalWrite()
 * 
 * Hardware:
 * - Arduino Uno R3 (ATmega328P @ 16 MHz)
 * - LED Built-in  : Pin D13 (terhubung ke Port B Bit 5 / PB5)
 * - Push Button   : Pin D8 (terhubung ke Port B Bit 0 / PB0) ke GND
 * 
 * Konsep Bitwise Register AVR:
 * - DDRB  (Data Direction Register B): Bit 1 = OUTPUT, Bit 0 = INPUT.
 * - PORTB (Port B Data Register): Bit 1 = HIGH / Aktifkan Pull-Up, Bit 0 = LOW.
 * - PINB  (Input Pins Register B): Membaca status pin. Menulis '1' ke PINB membalik (toggle) status pin.
 * 
 * Tujuan:
 * Mengukur selisih waktu 50.000 siklus switching pin antara fungsi bawaan Arduino Core
 * (yang sarat tabel lookup) dibandingkan manipulasi register 1 siklus CPU (62.5 ns).
 */

#include <Arduino.h>

const uint32_t ITERASI = 50000;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Tunggu koneksi serial USB terbuka
    }

    Serial.println(F("\n======================================================="));
    Serial.println(F("    BENCHMARK PORT MANIPULATION VS DIGITALWRITE()      "));
    Serial.println(F("======================================================="));
    Serial.println(F("Hardware: ATmega328P @ 16 MHz (1 siklus clock = 62.5 ns)"));
    Serial.print(F("Jumlah Pengujian: "));
    Serial.print(ITERASI);
    Serial.println(F(" iterasi switching pin D13 (PB5)\n"));

    // -------------------------------------------------------------------------
    // UJI 1: digitalWrite() bawaan Arduino Core
    // Memeriksa tabel lookup, mematikan timer PWM, dan menonaktifkan interupsi sesaat
    // -------------------------------------------------------------------------
    pinMode(13, OUTPUT);
    uint32_t mulaiStandard = micros();
    for (uint32_t i = 0; i < ITERASI; i++) {
        digitalWrite(13, HIGH);
        digitalWrite(13, LOW);
    }
    uint32_t durasiStandard = micros() - mulaiStandard;

    // -------------------------------------------------------------------------
    // UJI 2: Direct Register Manipulation (PORTB & DDRB)
    // Diterjemahkan langsung menjadi instruksi assembly silikon: 'sbi' dan 'cbi' (1 clock)
    // -------------------------------------------------------------------------
    DDRB |= (1 << PB5); // Konfigurasi PB5 (pin D13) sebagai OUTPUT via register arah
    uint32_t mulaiRegister = micros();
    for (uint32_t i = 0; i < ITERASI; i++) {
        PORTB |= (1 << PB5);  // Set bit 5 bernilai 1 (HIGH) dalam 1 siklus clock (62.5ns)
        PORTB &= ~(1 << PB5); // Set bit 5 bernilai 0 (LOW) dalam 1 siklus clock (62.5ns)
    }
    uint32_t durasiRegister = micros() - mulaiRegister;

    // -------------------------------------------------------------------------
    // Analisis Hasil Pengujian
    // -------------------------------------------------------------------------
    Serial.print(F("Total Waktu digitalWrite() : "));
    Serial.print(durasiStandard);
    Serial.println(F(" mikrodetik"));

    Serial.print(F("Total Waktu Direct Register: "));
    Serial.print(durasiRegister);
    Serial.println(F(" mikrodetik"));

    float rasio = (float)durasiStandard / (float)durasiRegister;
    Serial.print(F("Hasil                      : Direct Register "));
    Serial.print(rasio, 2);
    Serial.println(F("x LEBIH CEPAT!\n"));

    Serial.println(F("=== Mode Operasi Interaktif Register ==="));
    Serial.println(F("Tekan Tombol di Pin D8 (PB0) untuk toggle LED D13 (PB5) langsung di register"));

    // Konfigurasi pin D8 (PB0) sebagai INPUT_PULLUP menggunakan register murni:
    DDRB &= ~(1 << PB0); // DDRB bit 0 = 0 (Mode INPUT)
    PORTB |= (1 << PB0); // PORTB bit 0 = 1 (Aktifkan resistor PULL-UP internal)
}

void loop() {
    static bool statusSebelumnya = false;

    // Membaca status pin D8 secara instan melalui register PINB:
    // Hasil bitwise bernilai 0 saat tombol ditekan ke GND (Active-LOW)
    bool tombolDitekan = !(PINB & (1 << PB0));

    if (tombolDitekan && !statusSebelumnya) {
        // Trik AVR Hardware: Menulis bit 1 ke register PINx akan membalik (toggle)
        // logika pin output tanpa perlu membaca nilai PORTx sebelumnya!
        PINB |= (1 << PB5);
        Serial.println(F("[AKSI] Tombol PB0 ditekan -> Status LED PB5 dibalik via register PINB!"));
        delay(50); // Jeda software debounce sederhana
    }

    statusSebelumnya = tombolDitekan;
}
