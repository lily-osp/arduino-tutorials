// i2c_scanner.ino
// Modul 09: Memindai seluruh alamat pada bus I2C (Wire.h) dan menampilkan perangkat yang terdeteksi

#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(115200);
  while (!Serial);
  Serial.println(F("\n--- I2C Bus Scanner ---"));
}

void loop() {
  uint8_t jumlahPerangkat = 0;
  Serial.println(F("Memindai bus I2C..."));

  for (uint8_t alamat = 1; alamat < 127; alamat++) {
    Wire.beginTransmission(alamat);
    uint8_t error = Wire.endTransmission();

    if (error == 0) {
      Serial.print(F("Perangkat ditemukan pada alamat 0x"));
      if (alamat < 16) Serial.print(F("0"));
      Serial.print(alamat, HEX);
      Serial.println(F(" !"));
      jumlahPerangkat++;
    } else if (error == 4) {
      Serial.print(F("Error tidak dikenal pada alamat 0x"));
      if (alamat < 16) Serial.print(F("0"));
      Serial.println(alamat, HEX);
    }
  }

  if (jumlahPerangkat == 0) {
    Serial.println(F("Tidak ada perangkat I2C yang terdeteksi. Periksa kabel SDA/SCL."));
  } else {
    Serial.print(F("Selesai. Ditemukan "));
    Serial.print(jumlahPerangkat);
    Serial.println(F(" perangkat.\n"));
  }

  delay(5000);
}
