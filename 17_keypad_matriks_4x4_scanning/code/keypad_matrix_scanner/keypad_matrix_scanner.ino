/**
 * keypad_matrix_scanner.ino
 * Modul 17: Pemindaian Matriks Keypad 4x4 Mandiri (Row-Column Scanning Engine)
 * 
 * Hardware Wiring:
 * - Arduino Uno R3
 * - Keypad Matriks 4x4 (8 Pin Ribbon Header):
 *     * Pin 1 (Row 1) -> Pin D9 Uno (OUTPUT)
 *     * Pin 2 (Row 2) -> Pin D8 Uno (OUTPUT)
 *     * Pin 3 (Row 3) -> Pin D7 Uno (OUTPUT)
 *     * Pin 4 (Row 4) -> Pin D6 Uno (OUTPUT)
 *     * Pin 5 (Col 1) -> Pin D5 Uno (INPUT_PULLUP)
 *     * Pin 6 (Col 2) -> Pin D4 Uno (INPUT_PULLUP)
 *     * Pin 7 (Col 3) -> Pin D3 Uno (INPUT_PULLUP)
 *     * Pin 8 (Col 4) -> Pin D2 Uno (INPUT_PULLUP)
 * - LCD 1602 I2C: SDA -> Pin A4, SCL -> Pin A5, VCC -> 5V, GND -> GND
 * - Built-in LED: Pin D13 (PB5) untuk indikator kedip saat tombol ditekan
 * 
 * Prinsip Pemindaian Matriks:
 * 1. 4 Pin Baris dikonfigurasi sebagai OUTPUT (default HIGH / 5V).
 * 2. 4 Pin Kolom dikonfigurasi sebagai INPUT_PULLUP (terbaca HIGH saat sakelar terbuka).
 * 3. Algoritma menyalakan satu baris ke LOW secara bergantian, lalu membaca 4 pin kolom.
 *    Jika suatu kolom terbaca LOW, koordinat baris dan kolom tersebut menandakan tombol yang ditekan.
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const uint8_t JUMLAH_BARIS = 4;
const uint8_t JUMLAH_KOLOM = 4;

// Alokasi Pin Arduino untuk Baris dan Kolom
const uint8_t PIN_BARIS[JUMLAH_BARIS] = { 9, 8, 7, 6 }; // R1, R2, R3, R4
const uint8_t PIN_KOLOM[JUMLAH_KOLOM] = { 5, 4, 3, 2 }; // C1, C2, C3, C4

// Peta Karakter Matriks Tombol 4x4
const char PETA_KUNCI[JUMLAH_BARIS][JUMLAH_KOLOM] = {
    { '1', '2', '3', 'A' },
    { '4', '5', '6', 'B' },
    { '7', '8', '9', 'C' },
    { '*', '0', '#', 'D' }
};

// Variabel pelacak status dan debouncing matriks
char kunciTerakhir = '\0';
uint32_t waktuTekanTerakhir = 0;
const uint32_t JEDA_DEBOUNCE_KEYPAD = 60; // 60 milidetik filter getaran kontak membran

/**
 * Algoritma Pemindaian Matriks Murni (Tanpa Library Eksternal):
 * Mengembalikan karakter tombol jika ada tombol baru yang ditekan, atau '\0' jika idle.
 */
char pindaiKeypadMatriks() {
    char tombolTerdeteksi = '\0';

    for (uint8_t r = 0; r < JUMLAH_BARIS; r++) {
        // 1. Tarik baris aktif ke level LOW (0V)
        digitalWrite(PIN_BARIS[r], LOW);
        delayMicroseconds(5); // Waktu stabilisasi kapasitansi kabel (settling time)

        // 2. Baca seluruh kolom pada baris yang sedang aktif
        for (uint8_t c = 0; c < JUMLAH_KOLOM; c++) {
            if (digitalRead(PIN_KOLOM[c]) == LOW) {
                // Tombol pada koordinat [r][c] sedang terhubung ke GND!
                tombolTerdeteksi = PETA_KUNCI[r][c];
            }
        }

        // 3. Kembalikan baris ke level HIGH sebelum beralih ke baris berikutnya
        digitalWrite(PIN_BARIS[r], HIGH);

        if (tombolTerdeteksi != '\0') {
            break; // Hentikan pemindaian jika sudah menemukan tombol aktif
        }
    }

    return tombolTerdeteksi;
}

void setup() {
    Serial.begin(115200);

    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);

    // Inisialisasi 4 Pin Baris sebagai OUTPUT (Set ke HIGH agar tidak ada arus bocor)
    for (uint8_t r = 0; r < JUMLAH_BARIS; r++) {
        pinMode(PIN_BARIS[r], OUTPUT);
        digitalWrite(PIN_BARIS[r], HIGH);
    }

    // Inisialisasi 4 Pin Kolom sebagai INPUT dengan resistor internal PULL-UP
    for (uint8_t c = 0; c < JUMLAH_KOLOM; c++) {
        pinMode(PIN_KOLOM[c], INPUT_PULLUP);
    }

    Wire.begin();
    lcd.init();
    lcd.backlight();

    lcd.setCursor(0, 0);
    lcd.print(F("KEYPAD 4x4 SCAN "));
    lcd.setCursor(0, 1);
    lcd.print(F("TEKAN TOMBOL... "));

    Serial.println(F("\n=============================================="));
    Serial.println(F("   MODUL 17: PEMINDAIAN MATRIKS KEYPAD 4x4    "));
    Serial.println(F("=============================================="));
    Serial.println(F("Tekan sembarang tombol pada keypad 4x4..."));
}

void loop() {
    uint32_t sekarang = millis();

    // Jalankan pemindaian matriks
    char kunciSekarang = pindaiKeypadMatriks();

    // Logika Debouncing & State Filter: Deteksi transisi penekanan baru
    if (kunciSekarang != '\0' && kunciSekarang != kunciTerakhir) {
        if (sekarang - waktuTekanTerakhir >= JEDA_DEBOUNCE_KEYPAD) {
            waktuTekanTerakhir = sekarang;
            kunciTerakhir = kunciSekarang;

            // Indikator visual LED built-in menyala sekejap
            digitalWrite(13, HIGH);

            // Tampilkan ke Serial Monitor
            Serial.print(F("[KEYPAD EVENT] Tombol ditekan: '"));
            Serial.print(kunciSekarang);
            Serial.println(F("'"));

            // Tampilkan ke Layar LCD 1602
            lcd.setCursor(0, 1);
            char buffer[17];
            snprintf(buffer, sizeof(buffer), "Karakter: [ %c ] ", kunciSekarang);
            lcd.print(buffer);

            delay(30); // Durasi nyala LED built-in
            digitalWrite(13, LOW);
        }
    } 
    else if (kunciSekarang == '\0') {
        // Tombol telah dilepas oleh pengguna
        kunciTerakhir = '\0';
    }
}
