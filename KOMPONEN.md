# Daftar Kebutuhan Komponen: Hardware & Software

Panduan lengkap spesifikasi perangkat keras, perlengkapan sirkuit, perangkat lunak, pustaka (*library*), serta simulator yang dibutuhkan untuk mempraktikkan seluruh materi dari Modul 01 hingga Modul 18.

---

## 1. Perangkat Keras (Hardware Requirements)

Seluruh 18 modul dalam repositori ini dirancang secara terukur menggunakan komponen standar yang umum ditemukan pada paket belajar mikrokontroler.

### 1.1 Komponen Utama

| No | Komponen | Spesifikasi Teknis | Jumlah | Fungsi Utama dalam Proyek |
|---|---|---|---|---|
| 1 | **Arduino Uno R3** | Mikrokontroler ATmega328P DIP/SMD, Clock 16 MHz, Tegangan kerja 5V | 1 unit | Otak pemrosesan, pengendali timer, dan register |
| 2 | **LCD 1602 + I2C Backpack** | Layar matriks 16 kolom x 2 baris, backpack chip PCF8574 (alamat default `0x27` atau `0x3F`) | 1 unit | Menampilkan dashboard status, menu interaktif, dan prompt PIN |
| 3 | **Push Button Switch** | Tactile micro switch 4-pin (ukuran $6\times6\text{ mm}$ atau $12\times12\text{ mm}$) | 3 buah | Input navigasi menu, toggle relay, dan tombol darurat (E-STOP) |
| 4 | **Modul Relay 5V** | 1-Channel dengan optocoupler isolasi (Songle SRD-05VDC-SL-C), logika *Active-LOW*, terminal NO/NC/COM | 1 unit | Sakelar beban listrik, timer auto-cutoff, dan kunci solenoid |
| 5 | **LED 5mm** | Dioda pemancar cahaya (Merah, Hijau, atau Kuning), $V_f \approx 2.0\text{V}$, $I_f \approx 15\text{ mA}$ | 3 buah | Indikator visual digital, dimmer PWM, status akses, dan alarm |
| 6 | **Resistor Pembatas Arus** | $220\ \Omega$ (toleransi 5%, 1/4 Watt) | 3 buah | Melindungi LED dari kelebihan arus |
| 7 | **Buzzer Pasif (Piezo Transducer)** | Transduser keramik piezoelektrik tanpa osilator internal, impedansi $16\ \Omega$, respon 31 Hz – 65 kHz | 1 unit | Pembangkit nada audio, melodi jingle sukses, klik keypad, dan sirine |
| 8 | **Keypad Matriks 4x4** | Modul 16 tombol membran/tactile (angka 0-9, huruf A-D, simbol `*` dan `#`), 8-pin ribbon header | 1 unit | Input numerik sandi PIN keamanan dan pengetikan karakter |
| 9 | **Resistor Pembatas Buzzer** | $100\ \Omega$ (toleransi 5%, 1/4 Watt) | 1 buah | Meredam arus lonjakan dan melembutkan volume buzzer piezo |

---

### 1.2 Alat Pendukung & Jalur Perakitan

| No | Komponen Pendukung | Spesifikasi yang Disarankan | Fungsi |
|---|---|---|---|
| 1 | **Breadboard** | Half-size 400 titik atau Full-size 830 titik | Tempat merangkai sirkuit prototipe tanpa perlu menyolder |
| 2 | **Kabel Jumper Male-to-Male (M-M)** | Panjang 10–20 cm (minimal 15 helai) | Menghubungkan jalur breadboard ke header Arduino dan tombol |
| 3 | **Kabel Jumper Male-to-Female (M-F)** | Panjang 10–20 cm (minimal 15 helai) | Menghubungkan modul LCD I2C, Relay, dan Keypad ke Arduino |
| 4 | **Kabel USB Data** | Tipe USB-A ke USB-B (panjang 30–100 cm) | Suplai daya 5V dan transfer data serial upload program |

> **Peringatan Kabel USB**: Pastikan kabel USB yang kamu gunakan adalah **kabel transfer data**, bukan sekadar *charging cable*. Kabel pengisi daya murah tidak memiliki kabel tembaga jalur data (D+ dan D-), sehingga port Arduino tidak akan terdeteksi di komputer.

---

### 1.3 Catu Daya (Power Supply)

* **Praktik di Meja Kerja (Utama)**: Menggunakan daya 5V dari port USB komputer atau laptop melalui kabel USB Tipe A-ke-B ($500\text{ mA}$). Sangat mencukupi untuk menjalankan Uno, LCD, Relay, Keypad, dan Buzzer secara bersamaan.
* **Operasional Mandiri (*Standalone*, Opsional)**: Adaptor AC-DC $9\text{V}$ atau $12\text{V}$ ($1\text{A}$) dengan colokan *DC Barrel Jack* ukuran $5.5\times2.1\text{ mm}$ (pin tengah positif).

---

## 2. Perangkat Lunak & Toolchain (Software Requirements)

### 2.1 Lingkungan Pengembangan (IDE)

* **Arduino IDE 2.x (Rekomendasi Utama)**:
  * Unduh versi terbaru (v2.2.0 atau lebih baru) untuk Linux, Windows, atau macOS melalui situs resmi: [arduino.cc/en/software](https://www.arduino.cc/en/software).
  * Menyediakan fitur *auto-completion*, integrasi debugger, Serial Plotter interaktif, dan manajemen library terpadu.
* **PlatformIO / VS Code (Alternatif Tingkat Lanjut)**:
  * Bagi pengguna Visual Studio Code, seluruh contoh kode dapat langsung dikompilasi menggunakan extension PlatformIO IDE dengan target board `uno`.

---

### 2.2 Driver Komunikasi Serial USB

1. **Board Arduino Original (Chip ATmega16U2)**:
   * Terdeteksi otomatis tanpa driver tambahan di Linux (`/dev/ttyACM0`) dan macOS.
   * Di Windows, driver otomatis disertakan saat memasang Arduino IDE.
2. **Board Arduino Clone (Chip CH340 / CH341)**:
   * **Linux**: Modul kernel `ch341` sudah terpasang bawaan (terdeteksi sebagai `/dev/ttyUSB0`).
   * **Windows / macOS**: Membutuhkan instalasi driver WCH CH340 jika port COM belum muncul otomatis.
3. **Izin Akses Serial di Linux**:
   Pengguna Linux wajib memasukkan akun ke dalam grup sistem serial hardware:
   ```bash
   sudo usermod -a -G dialout $USER
   ```
   *(Setelah menjalankan perintah di atas, lakukan log out dan log in kembali)*.

---

### 2.3 Pustaka Tambahan (*Libraries*)

Hanya ada satu pustaka pihak ketiga yang perlu dipasang via Library Manager Arduino IDE:

* **`LiquidCrystal I2C`** (karya Frank de Brabander / Marco Schwartz):
  * Cara Pasang: Buka Arduino IDE $\rightarrow$ **Tools > Manage Libraries...** (atau `Ctrl + Shift + I`) $\rightarrow$ Ketik `LiquidCrystal I2C` $\rightarrow$ Klik **Install**.
  * Digunakan pada: Modul 07, 08, 09, 10, 15, 17, dan 18.

Semua pustaka lainnya merupakan pustaka standar bawaan arsitektur AVR/Arduino Core:
* `<Wire.h>`: Protokol komunikasi bus I2C.
* `<EEPROM.h>`: Baca dan tulis memori non-volatile internal ATmega328P.
* `<avr/wdt.h>`: Sirkuit proteksi Hardware Watchdog Timer.
* `<avr/sleep.h>`: Penghematan daya (*power-down sleep*).
* `<avr/interrupt.h>`: Vektor interupsi hardware silikon.

---

## 3. Simulator Online (Alternatif Tanpa Hardware Fisik)

Jika kamu belum memiliki komponen fisik, seluruh rangkaian dan program dapat disimulasikan 100% gratis di browser melalui **Wokwi Simulator**:

* **Website**: [wokwi.com](https://wokwi.com/)
* **Komponen yang Didukung Wokwi**:
  * Arduino Uno R3 (simulasi clock 16 MHz penuh).
  * Modul LCD 1602 I2C (mendukung CGRAM custom character).
  * Push Button tactile dengan pull-up internal.
  * Modul Relay 5V dengan animasi kontak NO/NC.
  * LED 5mm dan resistor presisi.
  * Buzzer pasif (audio browser nyata dengan `tone()`).
  * Membrane Keypad 4x4 dengan pemindaian matriks.
  * Emulasi pembacaan dan penulisan memori non-volatile EEPROM.

* **Cara Cepat Menjalankan Simulasi di Wokwi Web**:
  1. Buka proyek baru di [wokwi.com/arduino/new/uno](https://wokwi.com/arduino/new/uno).
  2. Klik tab `diagram.json` di editor Wokwi (atau tekan `F1` pilih diagram.json).
  3. Salin dan tempel isi file `diagram.json` dari folder modul praktikum. Sirkuit dan pengkabelan akan otomatis terangkai rapi di kanvas.
  4. Salin isi kode dari file `.ino` modul ke tab `sketch.ino`.
  5. Jika modul menggunakan LCD (lihat file `libraries.txt`), buka tab `Library Manager` di Wokwi dan tambahkan `LiquidCrystal I2C`.
  6. Klik tombol **Play** untuk memulai simulasi interaktif.

---

## 4. Matriks Distribusi Komponen per Modul (Modul 01 - 18)

| Modul | Uno R3 | LED | Tombol | Relay | LCD 1602 | Buzzer | Keypad 4x4 | Pustaka Eksternal | Simulasi Wokwi |
|---|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **Modul 01** (Anatomi Hardware) | Ya | - | - | - | - | - | - | Tidak ada | Teori |
| **Modul 02** (Arduino IDE Toolchain) | Ya | D13 | - | - | - | - | - | Tidak ada | [Wokwi #475523383202277377](https://wokwi.com/projects/475523383202277377) |
| **Modul 03** (Dasar Elektronika) | Ya | Ya (1x) | Ya (1x) | - | - | - | - | Tidak ada | Teori |
| **Modul 04** (Pemrograman C/C++) | Ya | - | - | - | - | - | - | Tidak ada | Teori |
| **Modul 05** (Digital I/O & Debounce)| Ya | Ya (1x) | Ya (1x) | - | - | - | - | Tidak ada | [Wokwi #475523604324977665](https://wokwi.com/projects/475523604324977665) |
| **Modul 06** (PWM & Dimmer) | Ya | Ya (1x) | Ya (2x) | - | - | - | - | Tidak ada | [Wokwi #475523980835133441](https://wokwi.com/projects/475523980835133441) |
| **Modul 07** (Serial & LCD I2C) | Ya | - | Ya (2x) | - | Ya | - | - | `LiquidCrystal I2C` | [Wokwi #475524767366901761](https://wokwi.com/projects/475524767366901761) |
| **Modul 08** (Kendali Relay 5V) | Ya | Ya (1x) | Ya (1x) | Ya | Ya | - | - | `LiquidCrystal I2C` | [Wokwi #475525208595155969](https://wokwi.com/projects/475525208595155969) |
| **Modul 09** (Bus I2C & EEPROM) | Ya | Ya (1x) | Ya (2x) | Ya | Ya | - | - | `LiquidCrystal I2C` | [diagram.json](09_bus_i2c_dan_eeprom/code/eeprom_state_persistence/diagram.json) |
| **Modul 10** (Capstone FSM & E-STOP) | Ya | Ya (1x) | Ya (3x) | Ya | Ya | - | - | `LiquidCrystal I2C` | [diagram.json](10_arsitektur_fsm_interrupt_capstone/code/smart_industrial_controller/diagram.json) |
| **Modul 11** (Direct Port Register) | Ya | D13 | Ya (1x) | - | - | - | - | Tidak ada | [diagram.json](11_port_manipulation_dan_register/code/direct_port_benchmark/diagram.json) |
| **Modul 12** (Timer1 CTC Interrupt) | Ya | D13 | - | Ya | - | - | - | Tidak ada | [diagram.json](12_timer_interrupt_dan_hardware_timers/code/timer1_precision_clock/diagram.json) |
| **Modul 13** (Low Power & Watchdog) | Ya | D13 | Ya (1x) | Ya | - | - | - | Tidak ada | [diagram.json](13_low_power_dan_watchdog_timer/code/power_saving_wdt/diagram.json) |
| **Modul 14** (Serial CLI Non-Blocking)| Ya | Ya (1x) | - | Ya | - | - | - | Tidak ada | [diagram.json](14_serial_cli_dan_command_parser/code/serial_cli_controller/diagram.json) |
| **Modul 15** (Advanced Capstone Station)| Ya | Ya (1x) | Ya (3x) | Ya | Ya | - | - | `LiquidCrystal I2C` | [diagram.json](15_sistem_menu_lcd_dan_capstone_lanjutan/code/advanced_industrial_station/diagram.json) |
| **Modul 16** (Frekuensi & Audio Buzzer)| Ya | Ya (1x) | Ya (1x) | - | - | Ya | - | Tidak ada | [diagram.json](16_pembangkit_frekuensi_dan_buzzer/code/buzzer_alarm_melody/diagram.json) |
| **Modul 17** (Keypad Matriks 4x4) | Ya | D13 | - | - | Ya | - | Ya | `LiquidCrystal I2C` | [diagram.json](17_keypad_matriks_4x4_scanning/code/keypad_matrix_scanner/diagram.json) |
| **Modul 18** (Sistem Keamanan PIN Access)| Ya | Ya (2x) | - | Ya | Ya | Ya | Ya | `LiquidCrystal I2C` | [diagram.json](18_sistem_keamanan_keypad_pin_access/code/keypad_security_access_controller/diagram.json) |
