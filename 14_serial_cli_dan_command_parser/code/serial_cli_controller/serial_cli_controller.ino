/**
 * serial_cli_controller.ino
 * Modul 14: Command Line Interface (CLI) & Telemetri Serial Non-Blocking
 * Hardware: Arduino Uno R3, LED PWM Pin D6, Modul Relay 5V Pin D7
 */

#include <Arduino.h>
#include <avr/wdt.h>

const uint8_t PIN_LED_PWM = 6;
const uint8_t PIN_RELAY   = 7;

const uint8_t UKURAN_BUFFER = 32;
char bufferSerial[UKURAN_BUFFER];
uint8_t indeksBuffer = 0;

bool statusRelay = false;
uint8_t nilaiPWM = 0;

void tampilkanBantuan() {
    Serial.println(F("\n--- PERINTAH SERIAL TERSEDIA ---"));
    Serial.println(F("  HELP             : Tampilkan menu ini"));
    Serial.println(F("  STATUS           : Cetak status sensor & aktuator (Format JSON)"));
    Serial.println(F("  RELAY:ON         : Menyalakan relay"));
    Serial.println(F("  RELAY:OFF        : Mematikan relay"));
    Serial.println(F("  RELAY:TOGGLE     : Membalik status relay"));
    Serial.println(F("  PWM:<0-255>      : Atur kecerahan LED (contoh: PWM:180)"));
    Serial.println(F("  RESET            : Reboot mikrokontroler via Watchdog"));
    Serial.println(F("--------------------------------\n"));
}

void cetakStatusJson() {
    Serial.print(F("{\"status\":\"ok\",\"uptime\":"));
    Serial.print(millis() / 1000);
    Serial.print(F(",\"relay\":"));
    Serial.print(statusRelay ? 1 : 0);
    Serial.print(F(",\"pwm\":"));
    Serial.print(nilaiPWM);
    Serial.println(F("}"));
}

void prosesPerintah(char* cmd) {
    // 1. Bersihkan whitespace / trailing spasi
    while (*cmd == ' ') cmd++;
    if (strlen(cmd) == 0) return;

    if (strcmp(cmd, "HELP") == 0) {
        tampilkanBantuan();
    } 
    else if (strcmp(cmd, "STATUS") == 0) {
        cetakStatusJson();
    } 
    else if (strcmp(cmd, "RELAY:ON") == 0) {
        statusRelay = true;
        digitalWrite(PIN_RELAY, LOW); // Active-LOW
        Serial.println(F("[OK] Relay Diaktifkan"));
    } 
    else if (strcmp(cmd, "RELAY:OFF") == 0) {
        statusRelay = false;
        digitalWrite(PIN_RELAY, HIGH); // Standby
        Serial.println(F("[OK] Relay Dimatikan"));
    } 
    else if (strcmp(cmd, "RELAY:TOGGLE") == 0) {
        statusRelay = !statusRelay;
        digitalWrite(PIN_RELAY, statusRelay ? LOW : HIGH);
        Serial.print(F("[OK] Relay Ditoggle -> "));
        Serial.println(statusRelay ? F("ON") : F("OFF"));
    } 
    else if (strncmp(cmd, "PWM:", 4) == 0) {
        int nilaiBaru = atoi(cmd + 4);
        if (nilaiBaru >= 0 && nilaiBaru <= 255) {
            nilaiPWM = (uint8_t)nilaiBaru;
            analogWrite(PIN_LED_PWM, nilaiPWM);
            Serial.print(F("[OK] Kecerahan LED PWM diatur ke: "));
            Serial.println(nilaiPWM);
        } else {
            Serial.println(F("[ERR] Nilai PWM tidak valid. Rentang yang diizinkan: 0 - 255"));
        }
    } 
    else if (strcmp(cmd, "RESET") == 0) {
        Serial.println(F("[WARN] Mereset Arduino melalui Watchdog..."));
        Serial.flush();
        wdt_enable(WDTO_15MS);
        while (true) { ; } // Masuk loop tak berujung, WDT akan mereset board dalam 15ms
    } 
    else {
        Serial.print(F("[ERR] Perintah tidak dikenal: '"));
        Serial.print(cmd);
        Serial.println(F("'. Ketik 'HELP' untuk bantuan."));
    }
}

void bacaSerialAsinkron() {
    while (Serial.available() > 0) {
        char c = (char)Serial.read();

        // Jika menerima karakter akhir baris (\n atau \r)
        if (c == '\n' || c == '\r') {
            if (indeksBuffer > 0) {
                bufferSerial[indeksBuffer] = '\0'; // Terminasi string C
                prosesPerintah(bufferSerial);
                indeksBuffer = 0; // Reset index buffer untuk baris selanjutnya
            }
        } 
        else {
            // Simpan karakter ke buffer jika masih ada ruang
            if (indeksBuffer < UKURAN_BUFFER - 1) {
                bufferSerial[indeksBuffer++] = c;
            } else {
                // Buffer overflow: buang dan reset untuk keamanan
                indeksBuffer = 0;
                Serial.println(F("[ERR] Buffer serial overflow! Maksimal 31 karakter."));
            }
        }
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(PIN_LED_PWM, OUTPUT);
    pinMode(PIN_RELAY, OUTPUT);

    digitalWrite(PIN_RELAY, HIGH); // Standby relay
    analogWrite(PIN_LED_PWM, 0);

    Serial.println(F("\n======================================"));
    Serial.println(F("ARDUINO SERIAL CLI & TELEMETRY ENGINE"));
    Serial.println(F("Baud Rate: 115200 | Ketik HELP untuk menu"));
    Serial.println(F("======================================"));
}

void loop() {
    bacaSerialAsinkron();
    // CPU tetap leluasa melakukan tugas lain di sini tanpa ada blocking sama sekali!
}
