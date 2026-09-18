/**
 * lcd1602_interactive.ino
 * Modul 07: Panel Kontrol Interaktif Menggunakan LCD 1602 I2C, Tombol, dan Serial UART
 * 
 * Hardware:
 * - Arduino Uno R3
 * - LCD 1602 + I2C Backpack (PCF8574):
 *     * VCC -> Pin 5V Uno
 *     * GND -> Pin GND Uno
 *     * SDA -> Pin A4 Uno (Jalur Data Serial I2C)
 *     * SCL -> Pin A5 Uno (Jalur Clock Serial I2C)
 * - Tombol UP   : Pin D2 ke GND (Active-LOW, INPUT_PULLUP)
 * - Tombol DOWN : Pin D3 ke GND (Active-LOW, INPUT_PULLUP)
 * 
 * Fitur:
 * 1. Menampilkan teks pada matriks 16 kolom x 2 baris via bus komunikasi 2 kabel (I2C).
 * 2. Karakter kustom panah atas dan bawah yang diunggah ke CGRAM LCD (alamat 0 dan 1).
 * 3. Sinkronisasi dua arah: nilai counter dapat diubah lewat tombol fisik maupun perintah Serial PC (+, -, r).
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Inisialisasi driver LCD pada alamat I2C 0x27 (atau 0x3F pada beberapa batch pabrikan)
// Format: LiquidCrystal_I2C(alamat_i2c, jumlah_kolom, jumlah_baris)
LiquidCrystal_I2C lcd(0x27, 16, 2);

const uint8_t PIN_BTN_UP   = 2;
const uint8_t PIN_BTN_DOWN = 3;

int counter = 0; // Nilai angka yang ditampilkan di panel

// Pelacak status debounce tombol
int statusUpTerakhir     = HIGH;
int statusDownTerakhir   = HIGH;
uint32_t waktuUpTerakhir   = 0;
uint32_t waktuDownTerakhir = 0;
const uint32_t JEDA_DEBOUNCE = 50;

// Matriks biner 5x8 piksel untuk membuat karakter kustom ikon panah atas
const uint8_t panahAtas[8] = {
    B00100,
    B01110,
    B10101,
    B00100,
    B00100,
    B00100,
    B00100,
    B00000
};

// Matriks biner 5x8 piksel untuk karakter kustom ikon panah bawah
const uint8_t panahBawah[8] = {
    B00100,
    B00100,
    B00100,
    B00100,
    B10101,
    B01110,
    B00100,
    B00000
};

void perbaruiTampilanLcd();

void setup() {
    pinMode(PIN_BTN_UP, INPUT_PULLUP);
    pinMode(PIN_BTN_DOWN, INPUT_PULLUP);

    Serial.begin(115200);
    Serial.println(F("\n=== PANEL INTERAKTIF LCD 1602 I2C ==="));
    Serial.println(F("Gunakan Tombol D2/D3 atau ketik '+', '-', 'r' di Serial Monitor"));

    // Inisialisasi bus I2C dan layar LCD
    lcd.init();
    lcd.backlight(); // Nyalakan lampu latar (backlight LED)

    // Simpan pola piksel ke memori karakter kustom (CGRAM):
    // Slot 0 untuk panah atas, Slot 1 untuk panah bawah
    lcd.createChar(0, (uint8_t*)panahAtas);
    lcd.createChar(1, (uint8_t*)panahBawah);

    // Gambar tampilan awal pada layar
    perbaruiTampilanLcd();
}

void loop() {
    uint32_t sekarang = millis();
    bool perluUpdateLcd = false;

    // -------------------------------------------------------------------------
    // 1. Baca Tombol UP (D2) dengan Debounce
    // -------------------------------------------------------------------------
    int bacaUp = digitalRead(PIN_BTN_UP);
    if (bacaUp != statusUpTerakhir) {
        waktuUpTerakhir = sekarang;
    }
    if ((sekarang - waktuUpTerakhir) > JEDA_DEBOUNCE) {
        static int statusUpStabil = HIGH;
        if (bacaUp != statusUpStabil) {
            statusUpStabil = bacaUp;
            if (statusUpStabil == LOW) {
                counter++;
                perluUpdateLcd = true;
            }
        }
    }
    statusUpTerakhir = bacaUp;

    // -------------------------------------------------------------------------
    // 2. Baca Tombol DOWN (D3) dengan Debounce
    // -------------------------------------------------------------------------
    int bacaDown = digitalRead(PIN_BTN_DOWN);
    if (bacaDown != statusDownTerakhir) {
        waktuDownTerakhir = sekarang;
    }
    if ((sekarang - waktuDownTerakhir) > JEDA_DEBOUNCE) {
        static int statusDownStabil = HIGH;
        if (bacaDown != statusDownStabil) {
            statusDownStabil = bacaDown;
            if (statusDownStabil == LOW) {
                counter--;
                perluUpdateLcd = true;
            }
        }
    }
    statusDownTerakhir = bacaDown;

    // -------------------------------------------------------------------------
    // 3. Baca Perintah Masuk dari Port Serial (Non-Blocking)
    // -------------------------------------------------------------------------
    while (Serial.available() > 0) {
        char karakter = (char)Serial.read();
        if (karakter == '+') {
            counter++;
            perluUpdateLcd = true;
        } else if (karakter == '-') {
            counter--;
            perluUpdateLcd = true;
        } else if (karakter == 'r' || karakter == 'R') {
            counter = 0;
            perluUpdateLcd = true;
        }
    }

    // -------------------------------------------------------------------------
    // 4. Perbarui Layar Hanya Jika Nilai Berubah (Mencegah Layar Berkedip/Flicker)
    // -------------------------------------------------------------------------
    if (perluUpdateLcd) {
        perbaruiTampilanLcd();
    }
}

void perbaruiTampilanLcd() {
    // Baris 0: Judul Panel & Simbol Panah Kustom
    lcd.setCursor(0, 0);
    lcd.print(F("PANEL KONTROL "));
    lcd.write(0); // Cetak karakter kustom slot 0 (panah atas)
    lcd.write(1); // Cetak karakter kustom slot 1 (panah bawah)

    // Baris 1: Nilai Counter
    lcd.setCursor(0, 1);
    // Format string dengan panjang tetap agar sisa karakter angka sebelumnya terhapus bersih
    char buffer[17];
    snprintf(buffer, sizeof(buffer), "Nilai: %-6d   ", counter);
    lcd.print(buffer);

    // Cetak log ke Serial Monitor untuk verifikasi
    Serial.print(F("Nilai Counter Terkini: "));
    Serial.println(counter);
}
