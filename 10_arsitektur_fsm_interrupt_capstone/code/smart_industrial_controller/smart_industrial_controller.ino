/**
 * smart_industrial_controller.ino
 * Modul 10 Capstone: Pengendali Industri Berbasis Finite State Machine (FSM),
 * Non-Blocking Scheduler, dan Hardware Interrupt E-STOP
 * 
 * Hardware Wiring:
 * 1. Tombol E-STOP (Darurat) : Pin D2 ke GND (INT0 Hardware Interrupt, Active-LOW)
 * 2. Tombol START / PAUSE    : Pin D3 ke GND (INPUT_PULLUP, Active-LOW)
 * 3. Tombol RESET ALARM      : Pin D4 ke GND (INPUT_PULLUP, Active-LOW)
 * 4. LED Status Multifungsi  : Pin D7 via Resistor 220 Ohm ke GND
 * 5. Modul Relay 5V (Beban)  : Pin D8 (Active-LOW: LOW = Nyala, HIGH = Standby)
 * 6. LCD 1602 I2C Backpack   : SDA -> A4, SCL -> A5, VCC -> 5V, GND -> GND
 * 
 * Arsitektur Sistem:
 * - FSM (Finite State Machine): Mengelola 3 status diskrit (STANDBY, RUNNING, EMERGENCY_STOP).
 * - Latensi Nol E-STOP: Begitu pin D2 jatuh ke LOW, ISR memotong daya relay dalam hitungan nanodetik
 *   pada tingkat hardware tanpa menunggu loop() menyelesaikan siklusnya.
 * - Logging EEPROM: Mencatat total siklus kerja sukses dan jumlah insiden darurat secara permanen.
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Definisi Pin Hardware
const uint8_t PIN_ESTOP_INT = 2;  // Jalur Hardware Interrupt INT0
const uint8_t PIN_BTN_START = 3;  // Tombol Mulai / Jeda
const uint8_t PIN_BTN_RESET = 4;  // Tombol Buka Kunci Alarm
const uint8_t PIN_LED       = 7;  // Indikator LED visual
const uint8_t PIN_RELAY     = 8;  // Modul Relay beban listrik

const uint8_t RELAY_ON  = LOW;  // Sinyal pemicu modul optocoupler relay
const uint8_t RELAY_OFF = HIGH; // Sinyal pemutus koil relay (standby)

// Status Diskrit FSM Sistem
enum class StatusFsm : uint8_t {
    STANDBY,        // Sistem diam menunggu tombol START
    RUNNING,        // Beban relay aktif bekerja selama siklus 10 detik
    EMERGENCY_STOP  // Sistem terkunci dalam kondisi darurat akibat tombol E-STOP
};

StatusFsm statusSistem = StatusFsm::STANDBY;

// Flag pemicu interupsi: Wajib bertipe volatile agar dibaca langsung dari RAM
volatile bool flagKedaruratan = false;

uint32_t waktuMulaiRunning   = 0;
const uint32_t DURASI_SIKLUS = 10000; // Durasi operasional siklus kerja (10 detik)

// Struktur pencatatan riwayat operasional ke EEPROM
struct LogIndustri {
    uint16_t magicNumber;
    uint32_t totalSiklusSukses;
    uint32_t totalInsidenEstop;
};

const uint16_t KODE_VALID  = 0xF5A1; // Penanda integritas blok data log
const int ALAMAT_EEPROM    = 0;
LogIndustri catatan;

// Pewaktu non-blocking untuk LED dan pembaruan LCD
uint32_t timerLed = 0;
uint32_t timerLcd = 0;
bool statusFisikLed = false;

// Pelacak status debounce tombol START dan RESET
int statusStartTerakhir = HIGH;
int statusResetTerakhir = HIGH;
uint32_t debounceStart  = 0;
uint32_t debounceReset  = 0;
const uint32_t JEDA_DEBOUNCE = 50;

void isrEmergencyStop();
void tanganiTransisiFsm(uint32_t sekarang);
void tanganiTombolOperasi(uint32_t sekarang);
void perbaruiLcdDashboard();
void kelolaIndikatorLed(uint32_t sekarang);
void simpanLogEeprom();

void setup() {
    // Inisialisasi pin relay ke posisi aman (OFF) sebelum mengaktifkan mode output
    digitalWrite(PIN_RELAY, RELAY_OFF);
    pinMode(PIN_RELAY, OUTPUT);

    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);

    pinMode(PIN_ESTOP_INT, INPUT_PULLUP);
    pinMode(PIN_BTN_START, INPUT_PULLUP);
    pinMode(PIN_BTN_RESET, INPUT_PULLUP);

    Serial.begin(115200);
    Serial.println(F("\n=========================================="));
    Serial.println(F("   SMART INDUSTRIAL CONTROLLER - CAPSTONE "));
    Serial.println(F("=========================================="));

    // Muat riwayat dari EEPROM
    EEPROM.get(ALAMAT_EEPROM, catatan);
    if (catatan.magicNumber != KODE_VALID) {
        catatan.magicNumber       = KODE_VALID;
        catatan.totalSiklusSukses = 0;
        catatan.totalInsidenEstop = 0;
        simpanLogEeprom();
    }

    // Pasang Hardware Interrupt pada Pin D2 (vektor INT0) pada transisi jatuh (FALLING: tombol ditekan)
    attachInterrupt(digitalPinToInterrupt(PIN_ESTOP_INT), isrEmergencyStop, FALLING);

    lcd.init();
    lcd.backlight();
    perbaruiLcdDashboard();
}

/**
 * Interrupt Service Routine (ISR) Darurat:
 * Hanya melakukan tugas super kritis: memotong daya relay seketika dan menaikkan flag.
 * Penulisan ke EEPROM atau LCD DILARANG di sini karena memakan waktu terlalu lama.
 */
void isrEmergencyStop() {
    digitalWrite(PIN_RELAY, RELAY_OFF); // Potong daya relay seketika!
    flagKedaruratan = true;
}

void loop() {
    uint32_t sekarang = millis();

    // -------------------------------------------------------------------------
    // 1. Tangani Flag Kedaruratan dari ISR
    // -------------------------------------------------------------------------
    if (flagKedaruratan) {
        flagKedaruratan = false;
        if (statusSistem != StatusFsm::EMERGENCY_STOP) {
            statusSistem = StatusFsm::EMERGENCY_STOP;
            catatan.totalInsidenEstop++;
            simpanLogEeprom(); // Catat insiden ke EEPROM secara aman di luar ISR
            Serial.println(F("[BAHAYA] HARDWARE EMERGENCY STOP TERPICU!"));
            perbaruiLcdDashboard();
        }
    }

    // -------------------------------------------------------------------------
    // 2. Eksekusi Mesin Status dan Input Tombol
    // -------------------------------------------------------------------------
    tanganiTombolOperasi(sekarang);
    tanganiTransisiFsm(sekarang);
    kelolaIndikatorLed(sekarang);

    // -------------------------------------------------------------------------
    // 3. Perbarui Tampilan LCD Setiap 250ms (Non-Blocking)
    // -------------------------------------------------------------------------
    if (sekarang - timerLcd >= 250) {
        timerLcd = sekarang;
        perbaruiLcdDashboard();
    }
}

void tanganiTransisiFsm(uint32_t sekarang) {
    switch (statusSistem) {
        case StatusFsm::STANDBY:
            digitalWrite(PIN_RELAY, RELAY_OFF);
            break;

        case StatusFsm::RUNNING:
            digitalWrite(PIN_RELAY, RELAY_ON);
            // Cek apakah siklus kerja 10 detik sudah selesai
            if (sekarang - waktuMulaiRunning >= DURASI_SIKLUS) {
                statusSistem = StatusFsm::STANDBY;
                digitalWrite(PIN_RELAY, RELAY_OFF);
                catatan.totalSiklusSukses++;
                simpanLogEeprom();
                Serial.println(F("[SELESAI] Siklus operasional selesai normal."));
                perbaruiLcdDashboard();
            }
            break;

        case StatusFsm::EMERGENCY_STOP:
            digitalWrite(PIN_RELAY, RELAY_OFF); // Pastikan relay selalu terputus
            break;
    }
}

void tanganiTombolOperasi(uint32_t sekarang) {
    // Pengecekan Tombol START / PAUSE (D3)
    int bacaStart = digitalRead(PIN_BTN_START);
    if (bacaStart != statusStartTerakhir) {
        debounceStart = sekarang;
    }
    if ((sekarang - debounceStart) > JEDA_DEBOUNCE) {
        static int statusStartStabil = HIGH;
        if (bacaStart != statusStartStabil) {
            statusStartStabil = bacaStart;
            if (statusStartStabil == LOW) {
                if (statusSistem == StatusFsm::STANDBY) {
                    statusSistem = StatusFsm::RUNNING;
                    waktuMulaiRunning = sekarang;
                    Serial.println(F("[OPERASI] Siklus RUNNING dimulai (10 detik)."));
                } else if (statusSistem == StatusFsm::RUNNING) {
                    statusSistem = StatusFsm::STANDBY;
                    Serial.println(F("[OPERASI] Siklus dijeda manual oleh operator."));
                }
                perbaruiLcdDashboard();
            }
        }
    }
    statusStartTerakhir = bacaStart;

    // Pengecekan Tombol RESET ALARM (D4)
    int bacaReset = digitalRead(PIN_BTN_RESET);
    if (bacaReset != statusResetTerakhir) {
        debounceReset = sekarang;
    }
    if ((sekarang - debounceReset) > JEDA_DEBOUNCE) {
        static int statusResetStabil = HIGH;
        if (bacaReset != statusResetStabil) {
            statusResetStabil = bacaReset;
            if (statusResetStabil == LOW) {
                // Hanya boleh reset jika tombol fisik E-STOP sudah dilepas kembali
                if (statusSistem == StatusFsm::EMERGENCY_STOP) {
                    if (digitalRead(PIN_ESTOP_INT) == HIGH) {
                        statusSistem = StatusFsm::STANDBY;
                        Serial.println(F("[RESET] Alarm dibersihkan. Sistem siap beroperasi."));
                        perbaruiLcdDashboard();
                    } else {
                        Serial.println(F("[GAGAL] Tombol fisik E-STOP masih tertahan ditekan!"));
                    }
                }
            }
        }
    }
    statusResetTerakhir = bacaReset;
}

void kelolaIndikatorLed(uint32_t sekarang) {
    uint32_t intervalKedip = 500; // Standar kedip mode STANDBY (1 Hz)

    if (statusSistem == StatusFsm::RUNNING) {
        digitalWrite(PIN_LED, HIGH); // LED menyala konstan saat relay aktif
        return;
    } else if (statusSistem == StatusFsm::EMERGENCY_STOP) {
        intervalKedip = 75; // Strobo kedip cepat tanda bahaya saat alarm trip
    }

    // Toggle non-blocking LED
    if (sekarang - timerLed >= intervalKedip) {
        timerLed = sekarang;
        statusFisikLed = !statusFisikLed;
        digitalWrite(PIN_LED, statusFisikLed ? HIGH : LOW);
    }
}

void perbaruiLcdDashboard() {
    char baris0[17];
    char baris1[17];

    switch (statusSistem) {
        case StatusFsm::STANDBY:
            snprintf(baris0, sizeof(baris0), "STATUS: STANDBY ");
            snprintf(baris1, sizeof(baris1), "D3:START Suk:%-3lu", catatan.totalSiklusSukses);
            break;

        case StatusFsm::RUNNING: {
            uint32_t waktuJalan = millis() - waktuMulaiRunning;
            int sisaDetik = (waktuJalan < DURASI_SIKLUS) ? (DURASI_SIKLUS - waktuJalan) / 1000 + 1 : 0;
            snprintf(baris0, sizeof(baris0), "STATUS: RUNNING ");
            snprintf(baris1, sizeof(baris1), "Sisa Waktu: %2ds  ", sisaDetik);
            break;
        }

        case StatusFsm::EMERGENCY_STOP:
            snprintf(baris0, sizeof(baris0), "!! E-STOP TRIP !");
            snprintf(baris1, sizeof(baris1), "D4:RESET Alm:%-3lu", catatan.totalInsidenEstop);
            break;
    }

    lcd.setCursor(0, 0);
    lcd.print(baris0);
    lcd.setCursor(0, 1);
    lcd.print(baris1);
}

void simpanLogEeprom() {
    EEPROM.put(ALAMAT_EEPROM, catatan);
}
