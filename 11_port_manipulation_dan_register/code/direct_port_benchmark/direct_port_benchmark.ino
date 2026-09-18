/**
 * direct_port_benchmark.ino
 * Modul 11: Benchmark Kecepatan Direct Port Manipulation vs digitalWrite()
 * Hardware: Arduino Uno R3, Built-in LED D13 (PB5), Push Button D8 (PB0)
 */

#include <Arduino.h>

const uint32_t ITERASI = 50000;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Tunggu koneksi serial
    }

    Serial.println(F("\n=== BENCHMARK PORT MANIPULATION VS DIGITALWRITE ==="));
    Serial.println(F("Hardware: ATmega328P @ 16 MHz"));
    Serial.print(F("Jumlah Pengujian: "));
    Serial.print(ITERASI);
    Serial.println(F(" iterasi switching pin D13 (PB5)\n"));

    // --- UJI 1: digitalWrite() bawaan Arduino Core ---
    pinMode(13, OUTPUT);
    uint32_t mulaiStandard = micros();
    for (uint32_t i = 0; i < ITERASI; i++) {
        digitalWrite(13, HIGH);
        digitalWrite(13, LOW);
    }
    uint32_t durasiStandard = micros() - mulaiStandard;

    // --- UJI 2: Direct Register Manipulation (PORTB & DDRB) ---
    DDRB |= (1 << PB5); // Set pin 13 (PB5) sebagai OUTPUT via register
    uint32_t mulaiRegister = micros();
    for (uint32_t i = 0; i < ITERASI; i++) {
        PORTB |= (1 << PB5);  // Set HIGH dalam 1 siklus clock
        PORTB &= ~(1 << PB5); // Set LOW dalam 1 siklus clock
    }
    uint32_t durasiRegister = micros() - mulaiRegister;

    // Cetak Hasil Analisis
    Serial.print(F("Waktu digitalWrite() : "));
    Serial.print(durasiStandard);
    Serial.println(F(" mikrodetik"));

    Serial.print(F("Waktu Direct Register: "));
    Serial.print(durasiRegister);
    Serial.println(F(" mikrodetik"));

    float rasio = (float)durasiStandard / (float)durasiRegister;
    Serial.print(F("Kecepatan Register   : "));
    Serial.print(rasio, 2);
    Serial.println(F("x LEBIH CEPAT!\n"));

    Serial.println(F("=== Mode Operasi Interaktif Register ==="));
    Serial.println(F("Tekan Tombol di Pin D8 (PB0) untuk toggle LED D13 (PB5)"));

    // Konfigurasi pin D8 (PB0) sebagai INPUT_PULLUP menggunakan register murni:
    DDRB &= ~(1 << PB0); // PB0 = INPUT (0)
    PORTB |= (1 << PB0); // PB0 = PULLUP aktif (1)
}

void loop() {
    static bool statusSebelumnya = false;

    // Baca status pin D8 secara langsung via PINB register:
    // Nilai bit 0 adalah 0 saat tombol ditekan (Active-LOW)
    bool tombolDitekan = !(PINB & (1 << PB0));

    if (tombolDitekan && !statusSebelumnya) {
        // Toggle LED D13 dengan menulis ke PINB register hardware:
        PINB |= (1 << PB5);
        Serial.println(F("[INTERAKSI] Tombol PB0 ditekan -> LED PB5 ditoggle via PINB!"));
        delay(50); // Software debounce sederhana untuk demo
    }

    statusSebelumnya = tombolDitekan;
}
