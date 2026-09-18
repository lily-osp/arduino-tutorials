/**
 * timer1_precision_clock.ino
 * Modul 12: Pembangkit Waktu Presisi Menggunakan Hardware Timer1 CTC Interrupt
 * 
 * Hardware:
 * - Arduino Uno R3 (ATmega328P @ 16 MHz)
 * - LED Indikator : Built-in LED pada pin D13 (PB5)
 * - Modul Relay 5V: Pin D7 (Active-LOW: 0V = ON, 5V = Standby)
 * 
 * Konsep Hardware Timer:
 * Timer1 adalah counter hardware 16-bit independen. Pada mode CTC (Clear Timer on Compare Match),
 * counter menghitung dari 0 hingga nilai register OCR1A (15624). Begitu cocok, perangkat keras
 * memicu ISR(TIMER1_COMPA_vect) tepat setiap 1.000 detik tanpa terpengaruh beban CPU di loop().
 */

#include <Arduino.h>

const uint8_t PIN_RELAY = 7;

// Variabel yang diubah di dalam ISR wajib bertipe volatile
volatile uint32_t detikBerjalan = 0;
volatile bool flagDetikBaru = false;

/**
 * ISR Timer1 Compare Match A:
 * Dipanggil secara otomatis oleh CPU tepat setiap 1.000 detik.
 */
ISR(TIMER1_COMPA_vect) {
    detikBerjalan++;
    flagDetikBaru = true;

    // Balik status LED D13 (PB5) langsung pada level register silikon
    PINB |= (1 << PB5);
}

void inisialisasiTimer1SatuDetik() {
    // 1. Nonaktifkan interupsi global sementara agar konfigurasi register tidak terinterupsi
    cli();

    // 2. Bersihkan seluruh register kontrol Timer 1
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0; // Setel penghitung awal ke angka 0

    // 3. Perhitungan Nilai Register OCR1A untuk Target 1.000 Hz (1 Detik):
    // Formula: OCR1A = (f_CPU / (Prescaler * Target_Hz)) - 1
    // OCR1A = (16.000.000 / (1024 * 1)) - 1 = 15.624
    OCR1A = 15624;

    // 4. Aktifkan mode CTC (Clear Timer on Compare Match):
    // Mode CTC diatur dengan menyalakan bit WGM12 pada register TCCR1B
    TCCR1B |= (1 << WGM12);

    // 5. Konfigurasi Prescaler ke 1024:
    // Kombinasi bit: CS12 = 1 dan CS10 = 1
    TCCR1B |= (1 << CS12) | (1 << CS10);

    // 6. Izinkan Interupsi Pembanding Timer 1 (Output Compare A Match Interrupt Enable):
    // Nyalakan bit OCIE1A pada register TIMSK1
    TIMSK1 |= (1 << OCIE1A);

    // 7. Aktifkan kembali interupsi global
    sei();
}

void setup() {
    Serial.begin(115200);

    pinMode(13, OUTPUT);
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, HIGH); // Standby relay (Active-LOW)

    Serial.println(F("\n=============================================="));
    Serial.println(F("    SISTEM PEWAKTU HARDWARE TIMER1 CTC        "));
    Serial.println(F("=============================================="));
    Serial.println(F("Clock: 16 MHz | Prescaler: 1024 | Target: 1.000 Hz"));
    Serial.println(F("Pewaktu berjalan di level hardware independen dari loop().\n"));

    inisialisasiTimer1SatuDetik();
}

void loop() {
    // Loop utama bebas menjalankan tugas panjang tanpa merusak akurasi detak waktu
    if (flagDetikBaru) {
        flagDetikBaru = false;

        uint32_t waktuLokal;
        // PENTING: Operasi Atomik pada Mikrokontroler 8-bit
        // Variabel uint32_t berukuran 4 byte. CPU 8-bit membutuhkan 4 instruksi untuk membacanya.
        // Kita nonaktifkan interupsi sesaat agar nilainya tidak terdistorsi jika ISR terpicu di tengah pembacaan.
        noInterrupts();
        waktuLokal = detikBerjalan;
        interrupts();

        Serial.print(F("[HARDWARE TICK] Detik ke-"));
        Serial.println(waktuLokal);

        // Contoh Aplikasi: Mengaktifkan relay setiap kelipatan 5 detik selama 2 detik
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
