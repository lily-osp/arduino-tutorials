# Daftar Kebutuhan Komponen: Hardware & Software

Panduan lengkap spesifikasi perangkat keras, perlengkapan sirkuit, perangkat lunak, pustaka (*library*), serta simulator yang dibutuhkan untuk mempraktikkan seluruh materi dari Modul 01 hingga Modul 15.

---

## 1. Perangkat Keras (Hardware Requirements)

Seluruh 15 modul dalam repositori ini dirancang secara efisien menggunakan **satu set 5 komponen standar**. Kamu tidak perlu membeli sensor mahal atau modul tambahan lain.

### 1.1 Komponen Utama

| No | Komponen | Spesifikasi Teknis | Jumlah | Fungsi Utama dalam Proyek |
|---|---|---|---|---|
| 1 | **Arduino Uno R3** | Mikrokontroler ATmega328P DIP/SMD, Clock 16 MHz, Tegangan kerja 5V | 1 unit | Otak pemrosesan, pengendali timer, dan register |
| 2 | **LCD 1602 + I2C Backpack** | Layar matriks 16 kolom x 2 baris, modul backpack PCF8574 (alamat default `0x27` atau `0x3F`) | 1 unit | Menampilkan antarmuka status, menu pengaturan, dan counter |
| 3 | **Push Button Switch** | Tactile micro switch 4-pin (ukuran $6\times6\text{ mm}$ atau $12\times12\text{ mm}$) | 3 buah | Input navigasi menu, toggle relay, dan tombol darurat (E-STOP) |
| 4 | **Modul Relay 5V** | 1-Channel dengan optocoupler isolasi (Songle SRD-05VDC-SL-C), logika *Active-LOW*, terminal NO/NC/COM | 1 unit | Sakelar kendali beban tegangan tinggi dan auto-cutoff timer |
| 5 | **LED 5mm** | Dioda pemancar cahaya (Merah, Hijau, atau Kuning), $V_f \approx 2.0\text{V}$, $I_f \approx 15\text{ mA}$ | 3 buah | Indikator visual digital, dimmer PWM, dan strobo darurat |
| 6 | **Resistor Pembatas Arus** | $220\ \Omega$ (toleransi 5%, daya 1/4 Watt, kode warna: Merah - Merah - Cokelat - Emas) | 3 buah | Melindungi LED dari kerusakan akibat arus berlebih |

---

### 1.2 Alat Pendukung & Jalur Perakitan

| No | Komponen Pendukung | Spesifikasi yang Disarankan | Fungsi |
|---|---|---|---|
| 1 | **Breadboard** | Half-size 400 titik atau Full-size 830 titik | Tempat merangkai sirkuit prototipe tanpa perlu menyolder |
| 2 | **Kabel Jumper Male-to-Male (M-M)** | Panjang 10–20 cm (minimal 15 helai) | Menghubungkan jalur breadboard ke header Arduino dan sakelar tombol |
| 3 | **Kabel Jumper Male-to-Female (M-F)** | Panjang 10–20 cm (minimal 10 helai) | Menghubungkan pin header modul LCD I2C dan Relay ke breadboard/Arduino |
| 4 | **Kabel USB Data** | Tipe USB-A ke USB-B (panjang 30–100 cm) | Suplai daya 5V dan jalur komunikasi serial upload sketch |

> ⚠️ **Peringatan Kabel USB**: Pastikan kabel USB yang kamu gunakan adalah **kabel transfer data**, bukan sekadar *charging cable*. Kabel pengisi daya murah sering kali tidak memiliki kabel tembaga jalur data (D+ dan D-) di dalamnya, sehingga komputer tidak akan pernah mendeteksi port Arduino.

---

### 1.3 Catu Daya (Power Supply)

* **Praktik di Meja Kerja (Utama)**: Cukup gunakan daya 5V dari port USB komputer atau laptop melalui kabel USB Tipe A-ke-B. Port USB 2.0/3.0 komputer mampu menyuplai arus hingga $500\text{ mA}$, sangat cukup untuk menyalakan Uno, LCD, dan satu relay secara bersamaan.
* **Operasional Mandiri (*Standalone*, Opsional)**: Adaptor AC-DC $9\text{V}$ atau $12\text{V}$ ($1\text{A}$) dengan colokan *DC Barrel Jack* ukuran $5.5\times2.1\text{ mm}$ (pin tengah positif).

---

## 2. Perangkat Lunak & Toolchain (Software Requirements)

### 2.1 Lingkungan Pengembangan (IDE)

* **Arduino IDE 2.x (Rekomendasi Utama)**:
  * Unduh versi terbaru (v2.2.0 atau lebih baru) untuk Linux, Windows, atau macOS melalui situs resmi: [arduino.cc/en/software](https://www.arduino.cc/en/software).
  * Menawarkan fitur *auto-completion*, integrasi debugger, Serial Plotter interaktif, dan manajemen library terpadu.
* **PlatformIO / VS Code (Alternatif Tingkat Lanjut)**:
  * Bagi pengguna yang terbiasa dengan ekosistem Visual Studio Code, seluruh contoh kode dapat langsung dikompilasi menggunakan extension PlatformIO IDE dengan board target `uno`.

---

### 2.2 Driver Komunikasi Serial USB

1. **Board Arduino Original (Chip ATmega16U2)**:
   * Terdeteksi otomatis tanpa driver tambahan di Linux (`/dev/ttyACM0`) dan macOS.
   * Di Windows, driver otomatis disertakan saat memasang Arduino IDE.
2. **Board Arduino Clone (Chip CH340 / CH341)**:
   * **Linux**: Modul kernel `ch341` sudah terpasang secara bawaan di hampir semua distribusi modern (terdeteksi sebagai `/dev/ttyUSB0`).
   * **Windows / macOS**: Membutuhkan instalasi driver WCH CH340 jika port COM tidak muncul otomatis.
3. **Izin Akses Serial di Linux**:
   Pengguna Linux wajib memasukkan akun pengguna ke dalam grup sistem yang berhak membaca perangkat port serial:
   ```bash
   sudo usermod -a -G dialout $USER
   ```
   *(Setelah menjalankan perintah di atas, lakukan log out dan log in kembali)*.

---

### 2.3 Pustaka Tambahan (*Libraries*)

Dari 15 modul yang ada, **hanya ada satu pustaka pihak ketiga** yang perlu kamu pasang via Library Manager:

* **`LiquidCrystal I2C`** (karya Frank de Brabander / Marco Schwartz):
  * Cara Pasang: Buka Arduino IDE $\rightarrow$ Klik menu **Tools > Manage Libraries...** (atau tekan `Ctrl + Shift + I`) $\rightarrow$ Ketik `LiquidCrystal I2C` $\rightarrow$ Klik tombol **Install**.
  * Digunakan pada: Modul 07, Modul 08, Modul 09, Modul 10, dan Modul 15.

Semua pustaka lainnya merupakan pustaka standar bawaan arsitektur AVR/Arduino yang sudah otomatis tersedia tanpa instalasi:
* `<Wire.h>`: Protokol komunikasi serial I2C (Two-Wire).
* `<EEPROM.h>`: Baca dan tulis memori non-volatile internal chip ATmega328P.
* `<avr/wdt.h>`: Pengendali sirkuit Hardware Watchdog Timer.
* `<avr/sleep.h>`: Manajemen mode penghematan daya (*power-down sleep*).
* `<avr/interrupt.h>`: Penanganan vektor interupsi perangkat keras tingkat silikon.

---

## 3. Simulator Online (Alternatif Tanpa Hardware Fisik)

Jika kamu belum memiliki komponen fisik di meja kerja, kamu tetap dapat menjalankan dan mempraktikkan seluruh materi serta kode program secara gratis melalui browser menggunakan **Wokwi Simulator**:

* **Website**: [wokwi.com](https://wokwi.com/)
* **Dukungan Komponen di Wokwi**:
  * Arduino Uno R3 (simulasi siklus instruksi clock kristal 16 MHz).
  * Modul LCD 1602 I2C (mendukung karakter kustom CGRAM).
  * Push Button dengan dukungan resistor pull-up internal.
  * Modul Relay 5V dengan indikasi visual kontak NO/NC.
  * LED 5mm aneka warna dan resistor presisi.
  * Emulasi pembacaan dan penulisan sel memori internal EEPROM.

---

## 4. Matriks Distribusi Komponen per Modul

Tabel berikut menunjukkan komponen apa saja yang aktif digunakan pada setiap modul:

| Modul | Uno R3 | LED + Resistor | Push Button | Modul Relay 5V | LCD 1602 I2C | Pustaka Eksternal |
|---|:---:|:---:|:---:|:---:|:---:|:---:|
| **Modul 01** (Anatomi Hardware) | ✔️ | - | - | - | - | Tidak ada |
| **Modul 02** (Arduino IDE Toolchain) | ✔️ | Built-in D13 | - | - | - | Tidak ada |
| **Modul 03** (Dasar Elektronika) | ✔️ | ✔️ (1x) | ✔️ (1x) | - | - | Tidak ada |
| **Modul 04** (Pemrograman C/C++) | ✔️ | - | - | - | - | Tidak ada |
| **Modul 05** (Digital I/O & Debounce)| ✔️ | ✔️ (1x) | ✔️ (1x) | - | - | Tidak ada |
| **Modul 06** (PWM & Dimmer) | ✔️ | ✔️ (1x) | ✔️ (2x) | - | - | Tidak ada |
| **Modul 07** (Serial & LCD I2C) | ✔️ | - | ✔️ (2x) | - | ✔️ | `LiquidCrystal I2C` |
| **Modul 08** (Kendali Relay 5V) | ✔️ | ✔️ (1x) | ✔️ (1x) | ✔️ | ✔️ | `LiquidCrystal I2C` |
| **Modul 09** (Bus I2C & EEPROM) | ✔️ | ✔️ (1x) | ✔️ (2x) | ✔️ | ✔️ | `LiquidCrystal I2C` |
| **Modul 10** (Capstone FSM & E-STOP) | ✔️ | ✔️ (1x) | ✔️ (3x) | ✔️ | ✔️ | `LiquidCrystal I2C` |
| **Modul 11** (Direct Port Register) | ✔️ | Built-in D13 | ✔️ (1x) | - | - | Tidak ada |
| **Modul 12** (Timer1 CTC Interrupt) | ✔️ | Built-in D13 | - | ✔️ | - | Tidak ada |
| **Modul 13** (Low Power & Watchdog) | ✔️ | Built-in D13 | ✔️ (1x) | ✔️ | - | Tidak ada |
| **Modul 14** (Serial CLI Non-Blocking)| ✔️ | ✔️ (1x) | - | ✔️ | - | Tidak ada |
| **Modul 15** (Advanced Capstone Station)| ✔️ | ✔️ (1x) | ✔️ (3x) | ✔️ | ✔️ | `LiquidCrystal I2C` |
