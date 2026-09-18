/**
 * blink_test.ino
 * Modul 02: Uji Coba Pertama Mengedipkan LED Built-in Pin 13
 * 
 * Hardware:
 * - Arduino Uno R3
 * - LED Built-in (sudah terpasang di board pada pin D13 via op-amp)
 * 
 * Alur Kerja:
 * Menguji apakah komputer berhasil mengenali port serial dan mengunggah binary program
 * ke chip ATmega328P. LED akan menyala selama 1 detik dan padam selama 1 detik berulang kali.
 */

#include <Arduino.h>

// Pin 13 terhubung ke indikator LED internal bertanda 'L' pada board Uno R3
const uint8_t PIN_LED = 13;

void setup() {
    // Konfigurasi pin D13 sebagai OUTPUT agar dapat menyuplai tegangan 5V
    pinMode(PIN_LED, OUTPUT);
}

void loop() {
    // Beri tegangan 5V ke anoda LED (LED menyala)
    digitalWrite(PIN_LED, HIGH);

    // Tahan eksekusi CPU selama 1000 milidetik (1 detik)
    delay(1000);

    // Turunkan tegangan ke 0V / GND (LED padam)
    digitalWrite(PIN_LED, LOW);

    // Tahan kembali selama 1 detik sebelum siklus loop() dimulai ulang
    delay(1000);
}
