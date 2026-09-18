/**
 * advanced_industrial_station.ino
 * Modul 15: Capstone Lanjutan - Sistem Otomasi Industri Terintegrasi
 * 
 * Hardware:
 * 1. Arduino Uno R3
 * 2. LCD 1602 I2C (SDA -> A4, SCL -> A5)
 * 3. Tombol 1 (Navigasi Menu): Pin D8 (INPUT_PULLUP)
 * 4. Tombol 2 (Pilih / Eksekusi): Pin D9 (INPUT_PULLUP)
 * 5. Tombol 3 (E-STOP Darurat): Pin D2 (INT0 Hardware Interrupt)
 * 6. Modul Relay 5V: Pin D7
 * 7. Indikator LED PWM: Pin D6 (+ Resistor 220 Ohm)
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <avr/wdt.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Definisi Pin
const uint8_t PIN_ESTOP       = 2; // INT0
const uint8_t PIN_LED_PWM     = 6;
const uint8_t PIN_RELAY       = 7;
const uint8_t PIN_BTN_NAV     = 8;
const uint8_t PIN_BTN_SELECT  = 9;

// Alamat EEPROM
const uint16_t ADDR_CONFIG_MAGIC = 0;
const uint16_t ADDR_TIMER_SEC    = 1;
const uint16_t ADDR_PWM_LEVEL    = 2;
const uint8_t  CONFIG_MAGIC_BYTE = 0x5C;

// State Sistem
enum ModeSistem {
    MODE_DASHBOARD,
    MODE_MENU,
    MODE_TIMING_ACTIVE,
    MODE_EMERGENCY
};

ModeSistem statusSaatIni = MODE_DASHBOARD;
volatile bool emergencyTriggered = false;

// Variabel Konfigurasi
uint8_t durasiTimerDetik = 10; // 5, 10, 30, 60
uint8_t tingkatPWM = 128;      // 0, 64, 128, 192, 255
bool statusRelay = false;
uint32_t waktuMulaiRelay = 0;

// Variabel Menu Navigasi
const uint8_t JUMLAH_MENU = 4;
uint8_t menuAktif = 0;
const char* const JUDUL_MENU[JUMLAH_MENU] = {
    "1.Relay Manual ",
    "2.Timer Durasi ",
    "3.LED Kecerahan",
    "4.Simpan EEPROM"
};

// ISR Hardware Emergency Stop
void isrEmergencyStop() {
    emergencyTriggered = true;
}

void muatKonfigurasiEEPROM() {
    if (EEPROM.read(ADDR_CONFIG_MAGIC) == CONFIG_MAGIC_BYTE) {
        durasiTimerDetik = EEPROM.read(ADDR_TIMER_SEC);
        tingkatPWM       = EEPROM.read(ADDR_PWM_LEVEL);
    } else {
        // Simpan nilai default jika EEPROM masih kosong
        EEPROM.update(ADDR_CONFIG_MAGIC, CONFIG_MAGIC_BYTE);
        EEPROM.update(ADDR_TIMER_SEC, durasiTimerDetik);
        EEPROM.update(ADDR_PWM_LEVEL, tingkatPWM);
    }
    analogWrite(PIN_LED_PWM, tingkatPWM);
}

void simpanKonfigurasiEEPROM() {
    EEPROM.update(ADDR_CONFIG_MAGIC, CONFIG_MAGIC_BYTE);
    EEPROM.update(ADDR_TIMER_SEC, durasiTimerDetik);
    EEPROM.update(ADDR_PWM_LEVEL, tingkatPWM);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("EEPROM Tersimpan"));
    lcd.setCursor(0, 1);
    lcd.print(F("Konfigurasi OK!"));
    delay(1000);
}

bool bacaTombolDebounced(uint8_t pin, bool &statusLama, uint32_t &waktuLama) {
    bool ditekan = false;
    int bacaanSekarang = digitalRead(pin);

    if (bacaanSekarang != statusLama) {
        waktuLama = millis();
    }

    if ((millis() - waktuLama) > 35) { // Debounce 35ms
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
    digitalWrite(PIN_RELAY, HIGH); // Standby relay

    Wire.begin();
    Wire.setWireTimeout(3000, true);
    lcd.init();
    lcd.backlight();

    // Attach Hardware Interrupt E-STOP
    attachInterrupt(digitalPinToInterrupt(PIN_ESTOP), isrEmergencyStop, FALLING);

    muatKonfigurasiEEPROM();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("INDUSTRIAL CTRL"));
    lcd.setCursor(0, 1);
    lcd.print(F("INITIALIZING... "));
    delay(800);

    wdt_enable(WDTO_2S); // Aktifkan WDT 2 detik
    updateLcdDashboard();
}

void loop() {
    wdt_reset(); // Beri makan Watchdog

    // 1. Cek Apakah Tombol Emergency Stop Terpicu
    if (emergencyTriggered) {
        statusSaatIni = MODE_EMERGENCY;
    }

    // Penanganan Darurat
    if (statusSaatIni == MODE_EMERGENCY) {
        digitalWrite(PIN_RELAY, HIGH); // Potong daya relay seketika!
        statusRelay = false;

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

        // Tekan Tombol Pilih untuk membuka kunci darurat
        if (digitalRead(PIN_BTN_SELECT) == LOW && digitalRead(PIN_ESTOP) == HIGH) {
            emergencyTriggered = false;
            statusSaatIni = MODE_DASHBOARD;
            analogWrite(PIN_LED_PWM, tingkatPWM);
            lcd.clear();
            updateLcdDashboard();
            delay(300);
        }
        return;
    }

    // 2. Debouncing Tombol Navigasi dan Select
    static bool navLama = HIGH, selLama = HIGH;
    static uint32_t tNav = 0, tSel = 0;
    bool btnNav = bacaTombolDebounced(PIN_BTN_NAV, navLama, tNav);
    bool btnSel = bacaTombolDebounced(PIN_BTN_SELECT, selLama, tSel);

    // 3. Mesin Status Antarmuka
    switch (statusSaatIni) {
        case MODE_DASHBOARD: {
            if (btnNav) {
                statusSaatIni = MODE_MENU;
                menuAktif = 0;
                lcd.clear();
            } else if (btnSel) {
                // Tombol Select di Dashboard = Pemicu Siklus Timer Relay
                statusRelay = true;
                digitalWrite(PIN_RELAY, LOW); // Aktifkan Relay
                waktuMulaiRelay = millis();
                statusSaatIni = MODE_TIMING_ACTIVE;
                lcd.clear();
            }
            break;
        }

        case MODE_TIMING_ACTIVE: {
            uint32_t detikBerjalan = (millis() - waktuMulaiRelay) / 1000;
            if (detikBerjalan >= durasiTimerDetik) {
                // Waktu timer habis
                digitalWrite(PIN_RELAY, HIGH);
                statusRelay = false;
                statusSaatIni = MODE_DASHBOARD;
                lcd.clear();
                updateLcdDashboard();
            } else {
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

            if (btnNav) {
                menuAktif = (menuAktif + 1) % JUMLAH_MENU;
            }

            if (btnSel) {
                if (menuAktif == 0) { // Toggle Manual Relay
                    statusRelay = !statusRelay;
                    digitalWrite(PIN_RELAY, statusRelay ? LOW : HIGH);
                } else if (menuAktif == 1) { // Ganti Durasi Timer
                    if (durasiTimerDetik == 5) durasiTimerDetik = 10;
                    else if (durasiTimerDetik == 10) durasiTimerDetik = 30;
                    else if (durasiTimerDetik == 30) durasiTimerDetik = 60;
                    else durasiTimerDetik = 5;
                } else if (menuAktif == 2) { // Ganti Nilai PWM
                    tingkatPWM = (tingkatPWM + 64) % 320;
                    if (tingkatPWM > 255) tingkatPWM = 0;
                    analogWrite(PIN_LED_PWM, tingkatPWM);
                } else if (menuAktif == 3) { // Simpan ke EEPROM & Keluar
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
