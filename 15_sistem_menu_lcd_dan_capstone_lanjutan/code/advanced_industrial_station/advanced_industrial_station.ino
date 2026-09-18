/**
 * advanced_industrial_station.ino
 * Modul 15: Capstone Lanjutan - Sistem Otomasi Industri Terintegrasi
 * 
 * Hardware Wiring:
 * 1. Arduino Uno R3
 * 2. LCD 1602 I2C Backpack   : SDA -> Pin A4, SCL -> Pin A5, VCC -> 5V, GND -> GND
 * 3. Tombol 1 (Navigasi Menu): Pin D8 ke GND (INPUT_PULLUP, Active-LOW)
 * 4. Tombol 2 (Pilih / Action): Pin D9 ke GND (INPUT_PULLUP, Active-LOW)
 * 5. Tombol 3 (E-STOP Darurat): Pin D2 ke GND (INT0 Hardware Interrupt, Active-LOW)
 * 6. Modul Relay 5V (Beban)  : Pin D7 (Active-LOW: 0V = ON, 5V = Standby)
 * 7. Indikator LED PWM       : Pin D6 via Resistor 220 Ohm ke GND
 * 
 * Arsitektur Terintegrasi:
 * - Antarmuka Menu LCD 2 Tingkat: Navigasi dan pemilihan parameter menggunakan 2 tombol.
 * - Non-Volatile EEPROM: Mengingat konfigurasi durasi timer dan kecerahan LED via update().
 * - I2C Anti-Hang: Mengaktifkan timeout bus I2C (Wire.setWireTimeout) agar LCD tidak membekukan CPU.
 * - Zero-Latency E-STOP: Hardware interrupt pada pin D2 memotong relay dalam hitungan nanodetik.
 * - Fault Tolerance: Watchdog Timer (WDT) 2.0 detik aktif untuk mengamankan mikrokontroler dari crash.
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <avr/wdt.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Definisi Pin Hardware
const uint8_t PIN_ESTOP       = 2; // Pin interrupt eksternal INT0
const uint8_t PIN_LED_PWM     = 6; // Pin berkemampuan PWM (Timer 0)
const uint8_t PIN_RELAY       = 7; // Pin kendali modul relay
const uint8_t PIN_BTN_NAV     = 8; // Tombol navigasi menu berikutnya (NEXT)
const uint8_t PIN_BTN_SELECT  = 9; // Tombol pilih opsi / mulai siklus timer (SELECT)

// Alamat Register Memori EEPROM
const uint16_t ADDR_CONFIG_MAGIC = 0; // Alamat penanda validitas
const uint16_t ADDR_TIMER_SEC    = 1; // Alamat penyimpanan durasi timer
const uint16_t ADDR_PWM_LEVEL    = 2; // Alamat penyimpanan kecerahan PWM
const uint8_t  CONFIG_MAGIC_BYTE = 0x5C; // Nilai kunci pembeda memori terformat

// Mesin Status Utama Sistem (FSM)
enum ModeSistem {
    MODE_DASHBOARD,     // Menampilkan status aktifitas dan informasi utama
    MODE_MENU,          // Mode navigasi konfigurasi parameter
    MODE_TIMING_ACTIVE, // Siklus hitung mundur aktivasi relay sedang berlangsung
    MODE_EMERGENCY      // Sistem terkunci dalam kondisi darurat akibat penekanan E-STOP
};

ModeSistem statusSaatIni = MODE_DASHBOARD;
volatile bool emergencyTriggered = false; // Flag interupsi perangkat keras

// Variabel Konfigurasi Sistem
uint8_t durasiTimerDetik = 10; // Pilihan durasi: 5, 10, 30, atau 60 detik
uint8_t tingkatPWM = 128;      // Tingkat kecerahan: 0, 64, 128, 192, 255
bool statusRelay = false;
uint32_t waktuMulaiRelay = 0;

// Struktur Menu Tampilan LCD
const uint8_t JUMLAH_MENU = 4;
uint8_t menuAktif = 0;
const char* const JUDUL_MENU[JUMLAH_MENU] = {
    "1.Relay Manual ",
    "2.Timer Durasi ",
    "3.LED Kecerahan",
    "4.Simpan EEPROM"
};

/**
 * ISR Hardware Emergency Stop (INT0):
 * Memotong suplai beban relay seketika di level silikon tanpa jeda CPU.
 */
void isrEmergencyStop() {
    emergencyTriggered = true;
}

void muatKonfigurasiEEPROM() {
    if (EEPROM.read(ADDR_CONFIG_MAGIC) == CONFIG_MAGIC_BYTE) {
        // Data valid ditemukan, pulihkan konfigurasi pengguna sebelumnya
        durasiTimerDetik = EEPROM.read(ADDR_TIMER_SEC);
        tingkatPWM       = EEPROM.read(ADDR_PWM_LEVEL);
    } else {
        // Inisialisasi awal jika EEPROM belum pernah ditulis
        EEPROM.update(ADDR_CONFIG_MAGIC, CONFIG_MAGIC_BYTE);
        EEPROM.update(ADDR_TIMER_SEC, durasiTimerDetik);
        EEPROM.update(ADDR_PWM_LEVEL, tingkatPWM);
    }
    analogWrite(PIN_LED_PWM, tingkatPWM);
}

void simpanKonfigurasiEEPROM() {
    // Gunakan update() agar hanya menulis ke sel silikon jika nilai berubah
    EEPROM.update(ADDR_CONFIG_MAGIC, CONFIG_MAGIC_BYTE);
    EEPROM.update(ADDR_TIMER_SEC, durasiTimerDetik);
    EEPROM.update(ADDR_PWM_LEVEL, tingkatPWM);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("EEPROM Tersimpan"));
    lcd.setCursor(0, 1);
    lcd.print(F("Konfigurasi OK! "));
    delay(800);
}

/**
 * Software Debounce Reusable:
 * Mengembalikan 'true' hanya saat transisi tombol baru saja stabil ditekan (Active-LOW).
 */
bool bacaTombolDebounced(uint8_t pin, bool &statusLama, uint32_t &waktuLama) {
    bool ditekan = false;
    int bacaanSekarang = digitalRead(pin);

    if (bacaanSekarang != statusLama) {
        waktuLama = millis();
    }

    if ((millis() - waktuLama) > 35) { // Jeda kestabilan 35 milidetik
        static int statusStabil[14];
        if (bacaanSekarang != statusStabil[pin]) {
            statusStabil[pin] = bacaanSekarang;
            if (statusStabil[pin] == LOW) {
                ditekan = true;
            }
        }
    }
    statusLama = bacaanSekarang;
    return ditekan;
}

void updateLcdDashboard() {
    lcd.setCursor(0, 0);
    lcd.print(F("STATUS: STANDBY "));
    lcd.setCursor(0, 1);
    lcd.print(F("R:"));
    lcd.print(statusRelay ? F("ON ") : F("OFF"));
    lcd.print(F(" T:"));
    lcd.print(durasiTimerDetik);
    lcd.print(F("s L:"));
    lcd.print(tingkatPWM);
    lcd.print(F("  "));
}

void setup() {
    Serial.begin(115200);

    pinMode(PIN_ESTOP, INPUT_PULLUP);
    pinMode(PIN_BTN_NAV, INPUT_PULLUP);
    pinMode(PIN_BTN_SELECT, INPUT_PULLUP);

    pinMode(PIN_RELAY, OUTPUT);
    pinMode(PIN_LED_PWM, OUTPUT);
    digitalWrite(PIN_RELAY, HIGH); // Standby relay (Active-LOW)

    // Inisialisasi bus I2C dengan proteksi timeout 3000 us (mencegah bus hang)
    Wire.begin();
    Wire.setWireTimeout(3000, true);
    
    lcd.init();
    lcd.backlight();

    // Pasang Hardware Interrupt untuk Tombol Darurat E-STOP
    attachInterrupt(digitalPinToInterrupt(PIN_ESTOP), isrEmergencyStop, FALLING);

    muatKonfigurasiEEPROM();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("INDUSTRIAL CTRL "));
    lcd.setCursor(0, 1);
    lcd.print(F("INITIALIZING... "));
    delay(800);

    // Aktifkan Watchdog Timer (timeout 2.0 detik)
    wdt_enable(WDTO_2S);
    updateLcdDashboard();
}

void loop() {
    // 1. Beri makan Watchdog Timer di setiap siklus eksekusi loop
    wdt_reset();

    // 2. Evaluasi apakah Tombol Darurat E-STOP ditekan
    if (emergencyTriggered) {
        statusSaatIni = MODE_EMERGENCY;
    }

    // -------------------------------------------------------------------------
    // STATUS KHUSUS: Penanganan Kondisi Darurat (EMERGENCY_STOP)
    // -------------------------------------------------------------------------
    if (statusSaatIni == MODE_EMERGENCY) {
        digitalWrite(PIN_RELAY, HIGH); // Pastikan relay selalu terputus!
        statusRelay = false;

        // Indikator strobo bahaya LED berkedip cepat (150ms)
        static uint32_t tBlink = 0;
        static bool ledState = false;
        if (millis() - tBlink >= 150) {
            tBlink = millis();
            ledState = !ledState;
            digitalWrite(PIN_LED_PWM, ledState ? HIGH : LOW);
        }

        lcd.setCursor(0, 0);
        lcd.print(F("*** E-STOP!  ***"));
        lcd.setCursor(0, 1);
        lcd.print(F("SISTEM DIKUNCI! "));

        // Buka kunci darurat hanya jika tombol fisik E-STOP dilepas dan operator menekan tombol SELECT
        if (digitalRead(PIN_BTN_SELECT) == LOW && digitalRead(PIN_ESTOP) == HIGH) {
            emergencyTriggered = false;
            statusSaatIni = MODE_DASHBOARD;
            analogWrite(PIN_LED_PWM, tingkatPWM); // Kembalikan kecerahan LED normal
            lcd.clear();
            updateLcdDashboard();
            delay(300); // Debounce sederhana pelepasan reset
        }
        return; // Hentikan eksekusi kode normal selama mode darurat aktif
    }

    // -------------------------------------------------------------------------
    // 3. Pembacaan Tombol Navigasi dan Seleksi dengan Software Debounce
    // -------------------------------------------------------------------------
    static bool navLama = HIGH, selLama = HIGH;
    static uint32_t tNav = 0, tSel = 0;
    bool btnNav = bacaTombolDebounced(PIN_BTN_NAV, navLama, tNav);
    bool btnSel = bacaTombolDebounced(PIN_BTN_SELECT, selLama, tSel);

    // -------------------------------------------------------------------------
    // 4. Mesin Status Antarmuka Pengguna (FSM UI)
    // -------------------------------------------------------------------------
    switch (statusSaatIni) {
        case MODE_DASHBOARD: {
            if (btnNav) {
                // Berpindah dari Dashboard ke Menu Pengaturan
                statusSaatIni = MODE_MENU;
                menuAktif = 0;
                lcd.clear();
            } else if (btnSel) {
                // Tombol Select di Dashboard memicu siklus kerja timer relay
                statusRelay = true;
                digitalWrite(PIN_RELAY, LOW); // Aktifkan beban relay
                waktuMulaiRelay = millis();
                statusSaatIni = MODE_TIMING_ACTIVE;
                lcd.clear();
            }
            break;
        }

        case MODE_TIMING_ACTIVE: {
            uint32_t detikBerjalan = (millis() - waktuMulaiRelay) / 1000;
            if (detikBerjalan >= durasiTimerDetik) {
                // Waktu operasional berakhir: Matikan beban dan kembali ke Dashboard
                digitalWrite(PIN_RELAY, HIGH);
                statusRelay = false;
                statusSaatIni = MODE_DASHBOARD;
                lcd.clear();
                updateLcdDashboard();
            } else {
                // Perbarui countdown sisa detik di layar LCD
                lcd.setCursor(0, 0);
                lcd.print(F("RELAY BEKERJA..."));
                lcd.setCursor(0, 1);
                lcd.print(F("Sisa: "));
                lcd.print(durasiTimerDetik - detikBerjalan);
                lcd.print(F(" detik   "));
            }
            break;
        }

        case MODE_MENU: {
            lcd.setCursor(0, 0);
            lcd.print(F("[PENGATURAN]    "));
            lcd.setCursor(0, 1);
            lcd.print(JUDUL_MENU[menuAktif]);

            // Tombol Navigasi: Menggulir ke opsi menu berikutnya
            if (btnNav) {
                menuAktif = (menuAktif + 1) % JUMLAH_MENU;
            }

            // Tombol Select: Menjalankan konfigurasi pada opsi yang sedang aktif
            if (btnSel) {
                if (menuAktif == 0) {
                    // Opsi 1: Toggle Manual Relay
                    statusRelay = !statusRelay;
                    digitalWrite(PIN_RELAY, statusRelay ? LOW : HIGH);
                } else if (menuAktif == 1) {
                    // Opsi 2: Ubah Durasi Timer (5s -> 10s -> 30s -> 60s)
                    if (durasiTimerDetik == 5) durasiTimerDetik = 10;
                    else if (durasiTimerDetik == 10) durasiTimerDetik = 30;
                    else if (durasiTimerDetik == 30) durasiTimerDetik = 60;
                    else durasiTimerDetik = 5;
                } else if (menuAktif == 2) {
                    // Opsi 3: Ubah Nilai PWM LED secara bertahap
                    tingkatPWM = (tingkatPWM + 64) % 320;
                    if (tingkatPWM > 255) tingkatPWM = 0;
                    analogWrite(PIN_LED_PWM, tingkatPWM);
                } else if (menuAktif == 3) {
                    // Opsi 4: Simpan Nilai ke EEPROM dan kembali ke Dashboard
                    simpanKonfigurasiEEPROM();
                    statusSaatIni = MODE_DASHBOARD;
                    lcd.clear();
                    updateLcdDashboard();
                }
            }
            break;
        }

        default:
            break;
    }
}
