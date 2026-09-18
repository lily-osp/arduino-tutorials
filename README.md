# Tutorial Arduino Lengkap: Dari Nol sampai Menengah

![Blotcat initializing an Arduino Uno circuit with a jumper wire and glowing LED](assets/blotcat_arduino_hero.jpg)

Panduan praktik pemrograman mikrokontroler dengan Arduino Uno R3. Materi disusun bertahap mulai dari pemahaman sirkuit board, instalasi lingkungan kerja, pemrograman C/C++, hingga pembuatan sistem otomasi berbasis Finite State Machine (FSM) dan interrupt.

Semua contoh sirkuit dan kode di tutorial ini dirancang khusus agar dapat dipraktikkan langsung menggunakan satu set komponen standar yang terjangkau.

---

## Hardware yang Digunakan

Kamu hanya membutuhkan 5 jenis komponen berikut untuk mencoba seluruh materi dari Modul 01 sampai Modul 10:

| Komponen | Tipe / Spesifikasi | Fungsi di Materi |
|---|---|---|
| **Arduino Uno R3** | Mikrokontroler ATmega328P (16 MHz, 5V) | Otak pemroses utama |
| **LCD 1602 + I2C** | 16 kolom x 2 baris dengan backpack PCF8574 | Menampilkan data dan menu |
| **Push Button** | Tactile switch 4 pin (2–3 buah) | Input tombol dan interupsi |
| **Modul Relay 5V** | 1 atau 2 channel dengan optocoupler | Mengendalikan beban listrik |
| **LED & Resistor** | LED 5mm (Merah, Hijau, Kuning) + Resistor $220\Omega$ | Indikator visual dan PWM |
| *Pendukung* | Breadboard 400/830 titik + Kabel Jumper (M-M, M-F) | Menyusun sirkuit tanpa solder |

> Kamu juga bisa menjalankan seluruh rangkaian secara virtual di browser menggunakan simulator [Wokwi](https://wokwi.com/) tanpa board fisik.

---

## Jalur Belajar (10 Modul)

Ikuti materi secara berurutan. Konsep di modul lanjutan mengandalkan pemahaman dari modul sebelumnya.

| Modul | Materi Pokok | Praktik yang Dibuat |
|---|---|---|
| [**Modul 01**](01_pengenalan_dan_hardware/README.md) | Sejarah Arduino, perbandingan board, dan anatomi sirkuit Uno R3 | Identifikasi komponen fisik dan batasan pin board |
| [**Modul 02**](02_arduino_ide_toolchain/README.md) | Arduino IDE 2.x, driver USB, Boards Manager, Serial Monitor, dan toolchain | Menyiapkan IDE, mendeteksi port, dan memahami proses upload |
| [**Modul 03**](03_dasar_elektronika/README.md) | Hukum Ohm, perhitungan resistor LED, cara kerja switch, dan proteksi relay | Menghitung nilai resistor dan merakit sirkuit aman |
| [**Modul 04**](04_pemrograman_embedded_cpp/README.md) | Siklus `setup()`/`loop()`, tipe data hemat memori, dan bahaya `String` | Menulis program C/C++ efisien untuk RAM 2KB |
| [**Modul 05**](05_digital_io_dan_debouncing/README.md) | Digital I/O, `INPUT_PULLUP`, contact bounce, dan software debouncing | Tombol tekan stabil untuk menyalakan LED tanpa getar mekanik |
| [**Modul 06**](06_pwm_dan_dimmer/README.md) | Prinsip kerja PWM, duty cycle, fungsi `analogWrite()`, dan timer | Pengatur kecerahan LED bertingkat dan efek breathing |
| [**Modul 07**](07_serial_dan_i2c_lcd1602/README.md) | Protokol UART, Serial Plotter, wiring I2C, dan library LCD 1602 | Mengirim perintah dari PC dan menampilkan karakter kustom di LCD |
| [**Modul 08**](08_kendali_relay/README.md) | Struktur modul relay 5V, optoisolator, logika Active-LOW, dan terminal NO/NC | Sakelar beban listrik aman dengan indikator status |
| [**Modul 09**](09_bus_i2c_dan_eeprom/README.md) | Scan alamat bus I2C (`Wire.h`) dan simpan status ke memori EEPROM internal | Menyimpan status sakelar agar tidak hilang saat mati lampu |
| [**Modul 10**](10_arsitektur_fsm_interrupt_capstone/README.md) | Non-blocking `millis()`, Finite State Machine, dan hardware interrupt | *Capstone*: Pengendali beban industri dengan mode timer dan tombol darurat |

---

## Cara Belajar yang Efektif

1. Baca penjelasan konsep dan skema pinout sebelum menyalakan board.
2. Rangkai komponen saat kabel USB belum tersambung ke komputer untuk menghindari risiko korsleting.
3. Ketik ulang kode program sendiri di Arduino IDE agar terbiasa dengan sintaks dan penanganan error.
4. Perhatikan pesan kesalahan di jendela output bawah Arduino IDE jika kode gagal dikompilasi atau gagal diunggah.

---

## Glosarium Istilah

Menemukan istilah teknis yang belum familiar seperti *optocoupler*, *pull-up resistor*, *duty cycle*, *contact bounce*, atau *Harvard Architecture*? Buka panduan istilah terpusat di:
👉 [**GLOSARIUM.md**](GLOSARIUM.md)

---

## Lisensi

Seluruh materi dan contoh kode bebas digunakan, dipelajari, dan dibagikan kembali di bawah lisensi [**MIT License**](LICENSE).

