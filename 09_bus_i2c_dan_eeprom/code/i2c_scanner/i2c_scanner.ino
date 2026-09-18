/**
 * i2c_scanner.ino
 * Modul 09: Pemindai Seluruh Alamat pada Bus Komunikasi I2C (Two-Wire Interface)
 * 
 * Hardware:
 * - Arduino Uno R3
 * - Perangkat I2C yang diuji (misal: LCD 1602 I2C):
 *     * SDA -> Pin A4 Uno
 *     * SCL -> Pin A5 Uno
 *     * VCC -> 5V Uno
 *     * GND -> GND Uno
 * 
 * Prinsip Kerja:
 * Bus I2C menggunakan pengalamatan 7-bit (rentang desimal 1 hingga 126).
 * Program mengirim sinyal START diikuti 7-bit alamat dan 1-bit WRITE.
 * Jika ada slave yang merespons dengan menarik jalur SDA ke LOW (bit ACK),
 * fungsi Wire.endTransmission() akan mengembalikan nilai 0.
 */

#include <Arduino.h>
#include <Wire.h>

void setup() {
    // Inisialisasi Arduino sebagai I2C Master
    Wire.begin();

    Serial.begin(115200);
    while (!Serial) {
        ; // Tunggu koneksi port serial (diperlukan untuk beberapa chip USB)
    }

    Serial.println(F("\n=========================================="));
    Serial.println(F("         ARDUINO I2C BUS SCANNER          "));
    Serial.println(F("=========================================="));
}

void loop() {
    uint8_t jumlahPerangkat = 0;
    Serial.println(F("Memindai bus I2C (alamat 0x01 s.d. 0x7E)..."));

    // Pindai seluruh rentang alamat 7-bit yang valid
    for (uint8_t alamat = 1; alamat < 127; alamat++) {
        // Mulai transaksi transmisi ke alamat target
        Wire.beginTransmission(alamat);
        
        // Akhiri transmisi dan tangkap kode status balik dari bus:
        // 0 = Success (ACK diterima, perangkat hadir)
        // 1 = Data terlalu panjang untuk buffer transmit
        // 2 = NACK diterima pada transmisi alamat (tidak ada slave)
        // 3 = NACK diterima pada transmisi data
        // 4 = Error bus lainnya (misal jalur short ke GND)
        uint8_t statusError = Wire.endTransmission();

        if (statusError == 0) {
            Serial.print(F(" -> Ditemukan perangkat I2C pada alamat: 0x"));
            if (alamat < 16) {
                Serial.print(F("0")); // Format padding 2 digit hex
            }
            Serial.print(alamat, HEX);
            Serial.println(F(" !"));
            jumlahPerangkat++;
        } else if (statusError == 4) {
            Serial.print(F(" -> Terdeteksi error fisik bus pada alamat: 0x"));
            if (alamat < 16) {
                Serial.print(F("0"));
            }
            Serial.println(alamat, HEX);
        }
    }

    if (jumlahPerangkat == 0) {
        Serial.println(F("[PERINGATAN] Tidak ada perangkat I2C yang terdeteksi."));
        Serial.println(F("             Periksa koneksi kabel SDA (A4) dan SCL (A5)."));
    } else {
        Serial.print(F("Pemindaian selesai. Total perangkat ditemukan: "));
        Serial.println(jumlahPerangkat);
    }

    Serial.println(F("Pemindaian ulang dalam 5 detik...\n"));
    delay(5000); // Jeda antar pemindaian
}
