/**
 * timer1_precision_clock.ino
 * Modul 12: Pembangkit Waktu Presisi Menggunakan Hardware Timer1 CTC Interrupt
 * Hardware: Arduino Uno R3, LED D13 (PB5), Modul Relay 5V Pin D7
 */

#include <Arduino.h>

const uint8_t PIN_RELAY = 7;
volatile uint32_t detikBerjalan = 0;
volatile bool flagDetikBaru = false;

// Interrupt Service Routine (ISR) dipanggil setiap tepat 1.000 detik oleh Timer1 hardware
ISR(TIMER1_COMPA_vect) {
    detikBerjalan++;
    flagDetikBaru = true;

    // Toggle LED D13 langsung di hardware register
    PINB |= (1 << PB5);
}

void inisialisasiTimer1SatuDetik() {
    // 1. Nonaktifkan interupsi global sementara
    cli();

    // 2. Reset register kontrol Timer1
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0; // Inisialisasi nilai counter awal ke 0

    // 3. Hitung nilai Compare Match untuk 1 Hz (1 detik):
    // Formula: OCR1A = (F_CPU / (Prescaler * Target_Frekuensi)) - 1
    // OCR1A = (16.000.000 / (1024 * 1)) - 1 = 15624
    OCR1A = 15624;

    // 4. Aktifkan CTC (Clear Timer on Compare Match) mode: WGM12 = 1
    TCCR1B |= (1 << WGM12);

    // 5. Set Prescaler ke 1024: CS12 = 1, CS10 = 1
    TCCR1B |= (1 << CS12) | (1 << CS10);

    // 6. Aktifkan Timer1 Compare Interrupt: OCIE1A = 1
    TIMSK1 |= (1 << OCIE1A);

    // 7. Aktifkan kembali interupsi global
    sei();
}

void setup() {
    Serial.begin(115200);

    pinMode(13, OUTPUT);
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, HIGH); // Standby (Active-LOW mati)

    Serial.println(F("\n=== SISTEM PEWAKTU HARDWARE TIMER1 CTC ==="));
    Serial.println(F("Clock ATmega328P: 16 MHz | Prescaler: 1024 | Target: 1.000 Hz"));
    Serial.println(F("ISR Timer1 berjalan independen tanpa dipengaruhi kode di loop()\n"));

    inisialisasiTimer1SatuDetik();
}

void loop() {
    // Loop utama bebas melakukan komputasi berat tanpa mengganggu akurasi waktu
    if (flagDetikBaru) {
        flagDetikBaru = false;

        uint32_t waktuLokal;
        // Salin variabel volatile dengan proteksi atomik
        noInterrupts();
        waktuLokal = detikBerjalan;
        interrupts();

        Serial.print(F("[TIMER TICK] Detik ke-"));
        Serial.println(waktuLokal);

        // Contoh: Aktifkan relay setiap kelipatan 5 detik selama 2 detik
        uint32_t sisaMod = waktuLokal % 5;
        if (sisaMod == 0) {
            digitalWrite(PIN_RELAY, LOW); // Relay AKTIF
            Serial.println(F(">>> RELAY: AKTIF (Siklus 5 Detik)"));
        } else if (sisaMod == 2) {
            digitalWrite(PIN_RELAY, HIGH); // Relay PADAM
            Serial.println(F(">>> RELAY: STANDBY"));
        }
    }
}
