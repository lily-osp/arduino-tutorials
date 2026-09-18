/**
 * serial_cli_controller.ino
 * Modul 14: Command Line Interface (CLI) & Telemetri Serial Non-Blocking
 * 
 * Hardware:
 * - Arduino Uno R3
 * - LED Indikator PWM : Pin D6 via Resistor 220 Ohm ke GND
 * - Modul Relay 5V    : Pin D7 (Active-LOW: 0V = ON, 5V = Standby)
 * - Kabel USB ke Komputer (Baud Rate: 115200)
 * 
 * Fitur:
 * 1. Non-blocking UART parser: Membaca karakter per karakter dari buffer hardware serial
 *    tanpa menggunakan fungsi blocking bawaan (seperti readString atau parseInt).
 * 2. C-String Safety: Menggunakan array statis char bufferSerial[32] untuk mencegah
 *    fragmentasi memori heap pada SRAM 2KB.
 * 3. Mesin Telemetri JSON: Perintah 'STATUS' menghasilkan satu baris JSON valid untuk integrasi IoT/Python.
 * 4. Software Reboot: Perintah 'RESET' mengeksekusi reboot hardware via Watchdog Timer 15ms.
 */

#include <Arduino.h>
#include <avr/wdt.h>

const uint8_t PIN_LED_PWM = 6; // Pin berkemampuan PWM untuk pengatur kecerahan LED
const uint8_t PIN_RELAY   = 7; // Pin kendali koil modul relay

// Buffer penerima data serial statis
const uint8_t UKURAN_BUFFER = 32;
char bufferSerial[UKURAN_BUFFER];
uint8_t indeksBuffer = 0;

// Variabel status operasional aktuator
bool statusRelay = false;
uint8_t nilaiPWM = 0;

void tampilkanBantuan() {
    Serial.println(F("\n--- DAFTAR PERINTAH SERIAL TERSEDIA ---"));
    Serial.println(F("  HELP             : Tampilkan daftar bantuan ini"));
    Serial.println(F("  STATUS           : Cetak status sensor & aktuator (Format JSON)"));
    Serial.println(F("  RELAY:ON         : Menyalakan beban relay"));
    Serial.println(F("  RELAY:OFF        : Mematikan beban relay"));
    Serial.println(F("  RELAY:TOGGLE     : Membalik status relay"));
    Serial.println(F("  PWM:<0-255>      : Atur kecerahan LED PWM (contoh: PWM:180)"));
    Serial.println(F("  RESET            : Reboot hardware Arduino via Watchdog"));
    Serial.println(F("----------------------------------------\n"));
}

void cetakStatusJson() {
    // Format JSON kompak satu baris untuk pembacaan otomatis oleh script Python / Node-RED
    Serial.print(F("{\"status\":\"ok\",\"uptime\":"));
    Serial.print(millis() / 1000);
    Serial.print(F(",\"relay\":"));
    Serial.print(statusRelay ? 1 : 0);
    Serial.print(F(",\"pwm\":"));
    Serial.print(nilaiPWM);
    Serial.println(F("}"));
}

void prosesPerintah(char* cmd) {
    // 1. Abaikan spasi awal jika ada
    while (*cmd == ' ') cmd++;
    if (strlen(cmd) == 0) return;

    // 2. Evaluasi perintah menggunakan perbandingan string C murni (strcmp / strncmp)
    if (strcmp(cmd, "HELP") == 0) {
        tampilkanBantuan();
    } 
    else if (strcmp(cmd, "STATUS") == 0) {
        cetakStatusJson();
    } 
    else if (strcmp(cmd, "RELAY:ON") == 0) {
        statusRelay = true;
        digitalWrite(PIN_RELAY, LOW); // Active-LOW: 0V menyalakan koil
        Serial.println(F("[OK] Relay Diaktifkan"));
    } 
    else if (strcmp(cmd, "RELAY:OFF") == 0) {
        statusRelay = false;
        digitalWrite(PIN_RELAY, HIGH); // Standby: 5V mematikan koil
        Serial.println(F("[OK] Relay Dimatikan"));
    } 
    else if (strcmp(cmd, "RELAY:TOGGLE") == 0) {
        statusRelay = !statusRelay;
        digitalWrite(PIN_RELAY, statusRelay ? LOW : HIGH);
        Serial.print(F("[OK] Relay Ditoggle -> "));
        Serial.println(statusRelay ? F("ON") : F("OFF"));
    } 
    else if (strncmp(cmd, "PWM:", 4) == 0) {
        // Ambil argumen angka di belakang prefix 'PWM:'
        int nilaiBaru = atoi(cmd + 4);
        if (nilaiBaru >= 0 && nilaiBaru <= 255) {
            nilaiPWM = (uint8_t)nilaiBaru;
            analogWrite(PIN_LED_PWM, nilaiPWM);
            Serial.print(F("[OK] Kecerahan LED PWM diatur ke: "));
            Serial.println(nilaiPWM);
        } else {
            Serial.println(F("[ERR] Nilai PWM tidak valid. Rentang yang diizinkan: 0 s.d 255"));
        }
    } 
    else if (strcmp(cmd, "RESET") == 0) {
        Serial.println(F("[WARN] Mereset Arduino melalui Watchdog Timer..."));
        Serial.flush(); // Tunggu hingga seluruh teks selesai terkirim
        
        // Trik Standar Industri: Setel WDT ke 15ms lalu kunci CPU dalam infinite loop.
        // Dalam 15 milidetik, hardware WDT akan mereset board secara bersih.
        wdt_enable(WDTO_15MS);
        while (true) {
            ; // Tunggu reboot
        }
    } 
    else {
        Serial.print(F("[ERR] Perintah tidak dikenal: '"));
        Serial.print(cmd);
        Serial.println(F("'. Ketik 'HELP' untuk bantuan."));
    }
}

/**
 * Membaca karakter yang masuk ke port serial secara asinkron tanpa menahan eksekusi CPU.
 */
void bacaSerialAsinkron() {
    while (Serial.available() > 0) {
        char c = (char)Serial.read();

        // Karakter akhir baris menandakan satu baris perintah telah lengkap
        if (c == '\n' || c == '\r') {
            if (indeksBuffer > 0) {
                bufferSerial[indeksBuffer] = '\0'; // Tambahkan terminator string C
                prosesPerintah(bufferSerial);
                indeksBuffer = 0; // Reset penunjuk index untuk perintah berikutnya
            }
        } 
        else {
            // Simpan karakter selama batas kapasitas buffer belum terlampaui
            if (indeksBuffer < UKURAN_BUFFER - 1) {
                bufferSerial[indeksBuffer++] = c;
            } else {
                // Proteksi Buffer Overflow: Buang baris dan beri peringatan
                indeksBuffer = 0;
                Serial.println(F("[ERR] Buffer serial overflow! Perintah melebihi 31 karakter."));
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

    Serial.println(F("\n=============================================="));
    Serial.println(F("    ARDUINO SERIAL CLI & TELEMETRY ENGINE     "));
    Serial.println(F("    Baud Rate: 115200 | Ketik HELP untuk menu "));
    Serial.println(F("=============================================="));
}

void loop() {
    // Jalankan mesin pembaca serial tanpa blocking
    bacaSerialAsinkron();

    // CPU tetap leluasa melakukan tugas lain di sini tanpa ada blocking
}
