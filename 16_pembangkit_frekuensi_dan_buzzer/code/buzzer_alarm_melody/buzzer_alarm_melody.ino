/**
 * buzzer_alarm_melody.ino
 * Modul 16: Pembangkit Frekuensi Nada Audio & Alarm Non-Blocking dengan Buzzer
 * 
 * Hardware Wiring:
 * 1. Arduino Uno R3
 * 2. Buzzer Pasif / Piezo Transducer:
 *    - Kaki Positif (+) -> Pin D11 (Pin PWM / Timer 2) via Resistor 100 Ohm (pembatas arus)
 *    - Kaki Negatif (-) -> Pin GND Uno
 * 3. Push Button (Pemilih Mode Suara):
 *    - Kaki 1 -> Pin D2 (INPUT_PULLUP, Active-LOW)
 *    - Kaki 2 -> Pin GND Uno
 * 4. LED Indikator Irama:
 *    - Anoda (+) via Resistor 220 Ohm -> Pin D6 (PWM)
 *    - Katoda (-) -> Pin GND Uno
 * 
 * Konsep Audio Frekuensi:
 * - Buzzer Pasif membutuhkan gelombang kotak periodik bolak-balik untuk menggetarkan diafragma keramik.
 * - Fungsi tone(pin, frekuensi, durasi) memanfaatkan Hardware Timer 2 mikrokontroler untuk
 *   menghasilkan pulsa frekuensi 31 Hz hingga 65.535 Hz secara mandiri tanpa membebani loop().
 */

#include <Arduino.h>

const uint8_t PIN_BUZZER = 11;
const uint8_t PIN_BUTTON = 2;
const uint8_t PIN_LED    = 6;

// Frekuensi nada musikal standar (Hertz)
#define NADA_C4  262
#define NADA_D4  294
#define NADA_E4  330
#define NADA_F4  349
#define NADA_G4  392
#define NADA_A4  440
#define NADA_B4  494
#define NADA_C5  523

// Melodi sukses / akses disetujui (nada, durasi_ms)
const uint16_t MELODI_SUKSES[] = { NADA_C4, NADA_E4, NADA_G4, NADA_C5 };
const uint16_t DURASI_SUKSES[] = { 100,     100,     100,     250     };

// Melodi error / penolakan
const uint16_t MELODI_ERROR[]  = { NADA_G4, NADA_D4, NADA_G4, NADA_D4 };
const uint16_t DURASI_ERROR[]  = { 150,     150,     150,     300     };

// Status sequencer suara non-blocking
enum ModeSuara {
    SUARA_IDLE,
    SUARA_BEEP_KLIK,
    SUARA_SUKSES,
    SUARA_ERROR,
    SUARA_SIRINE_ALARM
};

ModeSuara modeSuaraAktif = SUARA_IDLE;
uint8_t indeksNada = 0;
uint32_t waktuNadaTerakhir = 0;
uint16_t durasiNadaSaatIni = 0;

// Sirine sweep frekuensi
uint16_t frekuensiSirine = 600;
bool sirineMenaik = true;

// Debounce tombol
int statusTombolTerakhir = HIGH;
uint32_t waktuDebounce   = 0;
const uint32_t JEDA_DEBOUNCE = 50;
uint8_t pilihanEfek = 0; // 0=Beep, 1=Sukses, 2=Error, 3=Sirine

void putarEfekSuara(ModeSuara efek) {
    modeSuaraAktif = efek;
    indeksNada = 0;
    waktuNadaTerakhir = millis();
    durasiNadaSaatIni = 0;
    frekuensiSirine = 600;
    sirineMenaik = true;
}

void kelolaSequencerAudio(uint32_t sekarang) {
    switch (modeSuaraAktif) {
        case SUARA_IDLE:
            noTone(PIN_BUZZER);
            digitalWrite(PIN_LED, LOW);
            break;

        case SUARA_BEEP_KLIK:
            if (durasiNadaSaatIni == 0) {
                tone(PIN_BUZZER, 2400); // Bip pendek frekuensi tinggi 2.4 kHz
                digitalWrite(PIN_LED, HIGH);
                durasiNadaSaatIni = 40; // 40 milidetik
                waktuNadaTerakhir = sekarang;
            } else if (sekarang - waktuNadaTerakhir >= durasiNadaSaatIni) {
                noTone(PIN_BUZZER);
                digitalWrite(PIN_LED, LOW);
                modeSuaraAktif = SUARA_IDLE;
            }
            break;

        case SUARA_SUKSES:
            if (durasiNadaSaatIni == 0 || (sekarang - waktuNadaTerakhir >= durasiNadaSaatIni)) {
                if (indeksNada < 4) {
                    tone(PIN_BUZZER, MELODI_SUKSES[indeksNada]);
                    digitalWrite(PIN_LED, HIGH);
                    durasiNadaSaatIni = DURASI_SUKSES[indeksNada] + 20; // + jeda antarnada
                    waktuNadaTerakhir = sekarang;
                    indeksNada++;
                } else {
                    noTone(PIN_BUZZER);
                    digitalWrite(PIN_LED, LOW);
                    modeSuaraAktif = SUARA_IDLE;
                }
            }
            break;

        case SUARA_ERROR:
            if (durasiNadaSaatIni == 0 || (sekarang - waktuNadaTerakhir >= durasiNadaSaatIni)) {
                if (indeksNada < 4) {
                    tone(PIN_BUZZER, MELODI_ERROR[indeksNada]);
                    digitalWrite(PIN_LED, HIGH);
                    durasiNadaSaatIni = DURASI_ERROR[indeksNada] + 30;
                    waktuNadaTerakhir = sekarang;
                    indeksNada++;
                } else {
                    noTone(PIN_BUZZER);
                    digitalWrite(PIN_LED, LOW);
                    modeSuaraAktif = SUARA_IDLE;
                }
            }
            break;

        case SUARA_SIRINE_ALARM:
            // Sirine frekuensi menyapu (sweep) dari 600 Hz ke 1500 Hz
            if (sekarang - waktuNadaTerakhir >= 15) {
                waktuNadaTerakhir = sekarang;
                tone(PIN_BUZZER, frekuensiSirine);
                analogWrite(PIN_LED, map(frekuensiSirine, 600, 1500, 20, 255));

                if (sirineMenaik) {
                    frekuensiSirine += 30;
                    if (frekuensiSirine >= 1500) sirineMenaik = false;
                } else {
                    frekuensiSirine -= 30;
                    if (frekuensiSirine <= 600) {
                        sirineMenaik = true;
                        indeksNada++;
                        if (indeksNada >= 3) { // Bunyikan 3 siklus sapuan lalu selesai
                            noTone(PIN_BUZZER);
                            digitalWrite(PIN_LED, LOW);
                            modeSuaraAktif = SUARA_IDLE;
                        }
                    }
                }
            }
            break;
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_BUTTON, INPUT_PULLUP);

    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_LED, LOW);

    Serial.println(F("\n=============================================="));
    Serial.println(F("   MODUL 16: FREQUENCY GENERATOR & BUZZER     "));
    Serial.println(F("=============================================="));
    Serial.println(F("Tekan Tombol D2 untuk menggulir efek suara:"));
    Serial.println(F(" 1. Beep Klik Pendek (Feedback Tombol)"));
    Serial.println(F(" 2. Jingle Sukses / Akses Diterima"));
    Serial.println(F(" 3. Nada Peringatan Error / Akses Ditolak"));
    Serial.println(F(" 4. Sirine Darurat (Frequency Sweep)\n"));
}

void loop() {
    uint32_t sekarang = millis();

    // 1. Debounce Tombol Pemilih Efek Suara
    int bacaTombol = digitalRead(PIN_BUTTON);
    if (bacaTombol != statusTombolTerakhir) {
        waktuDebounce = sekarang;
    }
    if ((sekarang - waktuDebounce) > JEDA_DEBOUNCE) {
        static int statusStabil = HIGH;
        if (bacaTombol != statusStabil) {
            statusStabil = bacaTombol;
            if (statusStabil == LOW) {
                // Jalankan efek berikutnya
                if (pilihanEfek == 0) {
                    Serial.println(F("[EFEK] 1. Beep Klik Tombol"));
                    putarEfekSuara(SUARA_BEEP_KLIK);
                } else if (pilihanEfek == 1) {
                    Serial.println(F("[EFEK] 2. Nada Sukses (4 Chime)"));
                    putarEfekSuara(SUARA_SUKSES);
                } else if (pilihanEfek == 2) {
                    Serial.println(F("[EFEK] 3. Nada Error (Low Buzz)"));
                    putarEfekSuara(SUARA_ERROR);
                } else if (pilihanEfek == 3) {
                    Serial.println(F("[EFEK] 4. Sirine Darurat (Sweep Alarm)"));
                    putarEfekSuara(SUARA_SIRINE_ALARM);
                }

                pilihanEfek = (pilihanEfek + 1) % 4;
            }
        }
    }
    statusTombolTerakhir = bacaTombol;

    // 2. Jalankan sequencer audio secara non-blocking
    kelolaSequencerAudio(sekarang);
}
