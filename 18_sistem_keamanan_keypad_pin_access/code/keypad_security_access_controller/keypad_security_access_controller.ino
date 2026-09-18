/**
 * keypad_security_access_controller.ino
 * Modul 18: Sistem Keamanan Akses PIN Keypad 4x4, Kontrol Kunci Relay, dan Audio Alarm
 * 
 * Hardware Wiring:
 * 1. Keypad 4x4:
 *    - Baris R1..R4 -> Pin D9, D8, D7, D6 (OUTPUT)
 *    - Kolom C1..C4 -> Pin D5, D4, D3, D2 (INPUT_PULLUP)
 * 2. LCD 1602 I2C Backpack:
 *    - SDA -> Pin A4 Uno
 *    - SCL -> Pin A5 Uno
 *    - VCC -> 5V, GND -> GND
 * 3. Piezo Buzzer:
 *    - Positif (+) via Resistor 100 Ohm -> Pin D11
 *    - Negatif (-) -> GND
 * 4. Modul Relay 5V (Kunci Pintu Elektromagnetik / Solenoid):
 *    - Sinyal IN -> Pin D12 (Active-LOW: LOW = Kunci Terbuka, HIGH = Terkunci)
 *    - VCC -> 5V, GND -> GND
 * 5. LED Indikator:
 *    - LED Merah (Ditolak / Alarm) via 220 Ohm -> Pin D10
 *    - LED Hijau (Akses Diterima)  via 220 Ohm -> Pin D13 (Built-in PB5)
 * 
 * Fitur Sistem Keamanan:
 * - Masking PIN dengan simbol bintang (*) di layar LCD untuk kerahasiaan.
 * - Umpan balik audio (auditory feedback): Beep saat mengetik, nada sukses, nada error.
 * - Brute-Force Lockout: Salah 3 kali berturut-turut memicu sirine alarm dan mengunci sistem 30 detik.
 * - Penyimpanan PIN ke EEPROM: Penggantian PIN master tersimpan permanen.
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin Hardware
const uint8_t PIN_BUZZER    = 11;
const uint8_t PIN_RELAY     = 12;
const uint8_t PIN_LED_RED   = 10;
const uint8_t PIN_LED_GREEN = 13;

// Konfigurasi Matriks Keypad 4x4
const uint8_t JUMLAH_BARIS = 4;
const uint8_t JUMLAH_KOLOM = 4;
const uint8_t PIN_BARIS[JUMLAH_BARIS] = { 9, 8, 7, 6 };
const uint8_t PIN_KOLOM[JUMLAH_KOLOM] = { 5, 4, 3, 2 };

const char PETA_KUNCI[JUMLAH_BARIS][JUMLAH_KOLOM] = {
    { '1', '2', '3', 'A' },
    { '4', '5', '6', 'B' },
    { '7', '8', '9', 'C' },
    { '*', '0', '#', 'D' }
};

// Pengaturan PIN
const uint8_t PANJANG_PIN = 4;
char pinMaster[PANJANG_PIN + 1] = "1234"; // PIN default saat pertama kali
char bufferInputPin[PANJANG_PIN + 1];
uint8_t panjangInput = 0;

// Alamat EEPROM
const uint16_t ADDR_EEPROM_MAGIC = 0;
const uint16_t ADDR_EEPROM_PIN   = 2;
const uint16_t EEPROM_MAGIC_BYTE = 0xAA55;

// Status Mesin Keamanan (FSM)
enum StatusAkses {
    STATUS_STANDBY,
    STATUS_AKSES_DITERIMA,
    STATUS_AKSES_DITOLAK,
    STATUS_LOCKOUT_ALARM
};

StatusAkses statusSaatIni = STATUS_STANDBY;
uint8_t jumlahPercobaanSalah = 0;
uint32_t waktuMulaiAksi = 0;
const uint32_t DURASI_BUKA_PINTU_MS = 5000;   // Pintu terbuka 5 detik
const uint32_t DURASI_LOCKOUT_MS    = 30000;  // Sistem terkunci 30 detik jika salah 3x

// Deklarasi fungsi
char bacaKeypadNonBlocking();
void muatPinDariEeprom();
void simpanPinKeEeprom(const char* pinBaru);
void perbaruiTampilanStandby();
void bunyikanBipTombol();
void bunyikanNadaSukses();
void bunyikanNadaDitolak();

void setup() {
    Serial.begin(115200);

    // Inisialisasi Relay dalam posisi aman (Terkunci) sebelum mode OUTPUT
    digitalWrite(PIN_RELAY, HIGH);
    pinMode(PIN_RELAY, OUTPUT);

    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_LED_GREEN, OUTPUT);

    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_GREEN, LOW);

    // Inisialisasi pin baris keypad (OUTPUT HIGH)
    for (uint8_t r = 0; r < JUMLAH_BARIS; r++) {
        pinMode(PIN_BARIS[r], OUTPUT);
        digitalWrite(PIN_BARIS[r], HIGH);
    }
    // Inisialisasi pin kolom keypad (INPUT_PULLUP)
    for (uint8_t c = 0; c < JUMLAH_KOLOM; c++) {
        pinMode(PIN_KOLOM[c], INPUT_PULLUP);
    }

    Wire.begin();
    Wire.setWireTimeout(3000, true);
    lcd.init();
    lcd.backlight();

    muatPinDariEeprom();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("SECURITY SYSTEM "));
    lcd.setCursor(0, 1);
    lcd.print(F("INITIALIZING... "));
    delay(800);

    perbaruiTampilanStandby();
}

void loop() {
    uint32_t sekarang = millis();

    // -------------------------------------------------------------------------
    // 1. Tangani Status Terbuka (AKSES DITERIMA)
    // -------------------------------------------------------------------------
    if (statusSaatIni == STATUS_AKSES_DITERIMA) {
        if (sekarang - waktuMulaiAksi >= DURASI_BUKA_PINTU_MS) {
            // Waktu buka pintu selesai, kunci kembali relay
            digitalWrite(PIN_RELAY, HIGH); // Kunci relay
            digitalWrite(PIN_LED_GREEN, LOW);
            statusSaatIni = STATUS_STANDBY;
            panjangInput = 0;
            perbaruiTampilanStandby();
            Serial.println(F("[PINTU] Solenoid relay dikunci kembali."));
        }
        return;
    }

    // -------------------------------------------------------------------------
    // 2. Tangani Status Terkunci Total (LOCKOUT ALARM)
    // -------------------------------------------------------------------------
    if (statusSaatIni == STATUS_LOCKOUT_ALARM) {
        uint32_t waktuLockout = sekarang - waktuMulaiAksi;
        if (waktuLockout >= DURASI_LOCKOUT_MS) {
            // Masa hukuman lockout selesai
            noTone(PIN_BUZZER);
            digitalWrite(PIN_LED_RED, LOW);
            statusSaatIni = STATUS_STANDBY;
            jumlahPercobaanSalah = 0;
            panjangInput = 0;
            perbaruiTampilanStandby();
            Serial.println(F("[ALARM] Masa lockout selesai. Sistem kembali normal."));
        } else {
            // Strobo alarm dan sirine frekuensi bergantian
            static uint32_t tAlarm = 0;
            static bool nadaTinggi = false;
            if (sekarang - tAlarm >= 200) {
                tAlarm = sekarang;
                nadaTinggi = !nadaTinggi;
                tone(PIN_BUZZER, nadaTinggi ? 1200 : 700);
                digitalWrite(PIN_LED_RED, nadaTinggi ? HIGH : LOW);
            }

            lcd.setCursor(0, 0);
            lcd.print(F("! SISTEM DIKUNCI !"));
            lcd.setCursor(0, 1);
            char b[17];
            snprintf(b, sizeof(b), "Tunggu: %2lu dtk   ", (DURASI_LOCKOUT_MS - waktuLockout) / 1000 + 1);
            lcd.print(b);
        }
        return;
    }

    // -------------------------------------------------------------------------
    // 3. Baca Masukan Tombol Keypad
    // -------------------------------------------------------------------------
    char tombol = bacaKeypadNonBlocking();

    if (tombol != '\0') {
        bunyikanBipTombol();

        // Tombol '#' = SUBMIT PIN
        if (tombol == '#') {
            bufferInputPin[panjangInput] = '\0';

            if (panjangInput == PANJANG_PIN && strcmp(bufferInputPin, pinMaster) == 0) {
                // PIN COCOK: Akses Diterima
                statusSaatIni = STATUS_AKSES_DITERIMA;
                waktuMulaiAksi = sekarang;
                jumlahPercobaanSalah = 0;

                digitalWrite(PIN_RELAY, LOW); // Buka kunci relay (Active-LOW)
                digitalWrite(PIN_LED_GREEN, HIGH);
                digitalWrite(PIN_LED_RED, LOW);

                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print(F("AKSES DITERIMA! "));
                lcd.setCursor(0, 1);
                lcd.print(F("SILAKAN MASUK   "));

                Serial.println(F("[AKSES] PIN Benar! Kunci pintu dibuka 5 detik."));
                bunyikanNadaSukses();
            } else {
                // PIN SALAH: Akses Ditolak
                jumlahPercobaanSalah++;
                digitalWrite(PIN_LED_RED, HIGH);

                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print(F("! PIN SALAH !   "));
                lcd.setCursor(0, 1);
                char b[17];
                snprintf(b, sizeof(b), "Salah: %d dari 3  ", jumlahPercobaanSalah);
                lcd.print(b);

                Serial.print(F("[AKSES] PIN Ditolak! Percobaan salah ke-"));
                Serial.println(jumlahPercobaanSalah);
                bunyikanNadaDitolak();
                digitalWrite(PIN_LED_RED, LOW);

                panjangInput = 0;

                if (jumlahPercobaanSalah >= 3) {
                    // Masuk ke Mode Lockout Darurat
                    statusSaatIni = STATUS_LOCKOUT_ALARM;
                    waktuMulaiAksi = sekarang;
                    Serial.println(F("[PERINGATAN] 3x Percobaan salah! Sistem masuk LOCKOUT."));
                } else {
                    perbaruiTampilanStandby();
                }
            }
        }
        // Tombol '*' = BERSIHKAN / RESET INPUT
        else if (tombol == '*') {
            panjangInput = 0;
            perbaruiTampilanStandby();
        }
        // Tombol Angka (0-9, A-D): Tambah Karakter ke Buffer
        else {
            if (panjangInput < PANJANG_PIN) {
                bufferInputPin[panjangInput++] = tombol;
                
                // Tampilkan simbol bintang (*) di layar
                lcd.setCursor(5 + panjangInput, 1);
                lcd.print('*');
            }
        }
    }
}

char bacaKeypadNonBlocking() {
    static char kunciSebelumnya = '\0';
    static uint32_t waktuDebounceKeypad = 0;
    char kunciDitemukan = '\0';

    for (uint8_t r = 0; r < JUMLAH_BARIS; r++) {
        digitalWrite(PIN_BARIS[r], LOW);
        delayMicroseconds(5);

        for (uint8_t c = 0; c < JUMLAH_KOLOM; c++) {
            if (digitalRead(PIN_KOLOM[c]) == LOW) {
                kunciDitemukan = PETA_KUNCI[r][c];
            }
        }
        digitalWrite(PIN_BARIS[r], HIGH);
        if (kunciDitemukan != '\0') break;
    }

    if (kunciDitemukan != '\0' && kunciDitemukan != kunciSebelumnya) {
        if (millis() - waktuDebounceKeypad >= 50) {
            waktuDebounceKeypad = millis();
            kunciSebelumnya = kunciDitemukan;
            return kunciDitemukan;
        }
    } else if (kunciDitemukan == '\0') {
        kunciSebelumnya = '\0';
    }

    return '\0';
}

void perbaruiTampilanStandby() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("MASUKKAN PIN:   "));
    lcd.setCursor(0, 1);
    lcd.print(F("PIN: [____]     "));
}

void bunyikanBipTombol() {
    tone(PIN_BUZZER, 2200, 30); // Bip pendek 30ms
}

void bunyikanNadaSukses() {
    tone(PIN_BUZZER, 523, 100); delay(120); // C5
    tone(PIN_BUZZER, 659, 100); delay(120); // E5
    tone(PIN_BUZZER, 784, 250); delay(270); // G5
    noTone(PIN_BUZZER);
}

void bunyikanNadaDitolak() {
    tone(PIN_BUZZER, 200, 250); delay(280);
    tone(PIN_BUZZER, 180, 400); delay(420);
    noTone(PIN_BUZZER);
}

void muatPinDariEeprom() {
    uint16_t magic = 0;
    EEPROM.get(ADDR_EEPROM_MAGIC, magic);

    if (magic == EEPROM_MAGIC_BYTE) {
        for (uint8_t i = 0; i < PANJANG_PIN; i++) {
            pinMaster[i] = EEPROM.read(ADDR_EEPROM_PIN + i);
        }
        pinMaster[PANJANG_PIN] = '\0';
    } else {
        // Simpan default "1234"
        EEPROM.put(ADDR_EEPROM_MAGIC, EEPROM_MAGIC_BYTE);
        for (uint8_t i = 0; i < PANJANG_PIN; i++) {
            EEPROM.update(ADDR_EEPROM_PIN + i, pinMaster[i]);
        }
    }
}
