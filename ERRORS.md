# Panduan Pemecahan Masalah & Error Umum Arduino

Katalog pesan kesalahan, kegagalan upload, bug logika, dan masalah hardware yang paling sering ditemui saat memprogram Arduino Uno R3, lengkap dengan penyebab akar dan langkah solusinya.

---

## 1. Kegagalan Upload & Konektivitas USB

### `avrdude: stk500_recv(): programmer is not responding`
```text
avrdude: stk500_recv(): programmer is not responding
avrdude: stk500_getsync() attempt 1 of 10: not in sync: resp=0x00
```
* **Penyebab:**
  1. Komputer tidak dapat berkomunikasi dengan bootloader ATmega328P.
  2. Salah memilih port COM/serial pada menu **Tools > Port**.
  3. Ada kabel atau modul lain yang menancap pada pin **D0 (RX)** atau **D1 (TX)**. Pin ini terhubung langsung ke chip serial USB. Jika pin ini terhubung ke sensor/tombol, sinyal upload akan terdistorsi.
  4. Kabel USB hanya bertipe *charging-only* (tidak memiliki jalur data D+ dan D-).
* **Solusi:**
  1. Cabut sementara semua kabel jumper yang terhubung ke pin 0 (RX) dan 1 (TX).
  2. Buka **Tools > Port**, pastikan port yang dipilih memiliki tanda centang dan nama board.
  3. Ganti kabel USB dengan kabel data yang sudah teruji.
  4. Jika menggunakan Arduino clone (chip CH340), pastikan driver CH340 sudah terpasang dan terdeteksi pada sistem operasi (`lsusb` di Linux atau Device Manager di Windows).

---

### `avrdude: ser_open(): can't open device "/dev/ttyACM0": Permission denied`
```text
avrdude: ser_open(): can't open device "/dev/ttyACM0": Permission denied
Failed uploading: uploading error: exit status 1
```
* **Penyebab:** Pengguna di sistem operasi Linux belum dimasukkan ke dalam grup sistem yang berhak mengakses port serial hardware (`dialout` atau `uucp`).
* **Solusi:**
  Jalankan perintah berikut di terminal Linux kamu:
  ```bash
  sudo usermod -a -G dialout $USER
  ```
  *(Jika menggunakan Arch Linux / Manjaro, gunakan grup `uucp`: `sudo usermod -a -G uucp $USER`)*.  
  Setelah itu, **log out** dari desktop dan masuk kembali agar perubahan izin grup diterapkan.

---

### Port Serial Tidak Muncul di Arduino IDE
* **Penyebab:**
  1. Chip USB interface (ATmega16U2 atau CH340) tidak mendapat daya atau driver belum aktif.
  2. Konektor USB longgar atau port USB komputer mengalami proteksi overcurrent.
* **Solusi:**
  1. Cek apakah lampu LED indikator **ON** di board Arduino menyala hijau/merah terang.
  2. Di Linux, jalankan perintah terminal:
     ```bash
     dmesg -w
     ```
     Lalu colokkan kabel USB. Terminal harus mencetak baris deteksi seperti `ch340-uart converter now attached to ttyUSB0` atau `cdc_acm ... ttyACM0`.
  3. Coba pindahkan ke port USB lain di motherboard langsung (hindari USB hub pasif tanpa adaptor daya tambahan).

---

## 2. Kesalahan Kompilasi (Compiler Errors)

### `fatal error: LiquidCrystal_I2C.h: No such file or directory`
```text
sketch.ino:1:10: fatal error: LiquidCrystal_I2C.h: No such file or directory
 #include <LiquidCrystal_I2C.h>
          ^~~~~~~~~~~~~~~~~~~~~
compilation terminated.
```
* **Penyebab:** Library yang dipanggil di baris `#include` belum terpasang di Arduino IDE.
* **Solusi:**
  1. Buka Arduino IDE, klik ikon **Library Manager** di bilah sisi kiri (atau tekan `Ctrl + Shift + I`).
  2. Ketik `LiquidCrystal I2C` di kolom pencarian.
  3. Cari library karya **Frank de Brabander** atau **Marco Schwartz**, lalu klik **Install**.
  4. Kompilasi ulang kode kamu (`Ctrl + R`).

---

### `redefinition of 'void setup()'` atau Variabel Ganda
```text
sketch.ino:15:6: error: redefinition of 'void setup()'
 void setup() {
      ^~~~~
sketch.ino:1:6: note: 'void setup()' previously defined here
```
* **Penyebab:** Ada dua fungsi `setup()` atau `loop()` di dalam satu project sketch, atau terdapat beberapa file `.ino` di folder yang sama yang mendeklarasikan nama variabel/fungsi global yang sama. Arduino IDE secara otomatis menggabungkan seluruh file `.ino` di satu folder sebelum mengompilasinya.
* **Solusi:**
  1. Pastikan dalam satu folder sketch hanya terdapat satu fungsi `void setup()` dan satu fungsi `void loop()`.
  2. Pisahkan sketch eksperimen lain ke folder proyek terpisah.

---

### Dynamic Memory Warning: Out of Memory (SRAM Habis)
```text
Global variables use 2140 bytes (104%) of dynamic memory, leaving -92 bytes for local variables.
Maximum is 2048 bytes.
Sketch too big; see https://support.arduino.cc/ for tips on reducing it.
```
* **Penyebab:** Uno R3 hanya memiliki SRAM sebesar 2.048 byte (2 KB). Deklarasi array besar, teks string panjang tanpa makro `F()`, atau penggunaan objek `String` berlebihan akan menghabiskan ruang memori ini.
* **Solusi:**
  1. Bungkus semua string statis di `Serial.print()` dengan makro `F()`, contoh:
     ```cpp
     // SEBELUM (memakan RAM):
     Serial.println("Sistem Otomasi Aktif Menunggu Perintah...");
     // SESUDAH (tersimpan di Flash memory, RAM aman):
     Serial.println(F("Sistem Otomasi Aktif Menunggu Perintah..."));
     ```
  2. Ganti tipe data variabel: gunakan `uint8_t` (1 byte) untuk nomor pin atau angka $0-255$, jangan gunakan `int` (2 byte) atau `long` (4 byte) jika tidak perlu.
  3. Ganti objek `String` dengan array karakter statis `char buf[20]` (lihat Modul 04).

---

### `stray '\302' in program` atau `stray '\...`
```text
sketch.ino:12:1: error: stray '\302' in program
sketch.ino:12:2: error: stray '\240' in program
```
* **Penyebab:** Kamu menyalin kode dari halaman web atau dokumen yang mengandung karakter spasi tak kasat mata (*non-breaking space*, Unicode `U+00A0`), kutip miring (curly quotes `”`), atau minus panjang (em-dash).
* **Solusi:**
  1. Periksa baris yang ditunjukkan pada pesan error.
  2. Hapus spasi di awal atau akhir baris tersebut, lalu ketik ulang spasinya menggunakan tombol Spasi standar di keyboard.
  3. Pastikan tanda petik yang digunakan adalah tanda petik lurus (`"` atau `'`).

---

## 3. Masalah Komunikasi Serial & I2C

### Serial Monitor Menampilkan Karakter Sampah (Garbled Text: `⸮⸮⸮`)
* **Penyebab:** Nilai baud rate yang diatur pada `Serial.begin(...)` di kode Arduino tidak sama dengan baud rate yang dipilih pada menu dropdown Serial Monitor.
* **Solusi:**
  1. Buka file `.ino`, periksa baris:
     ```cpp
     Serial.begin(115200);
     ```
  2. Buka jendela **Serial Monitor** di Arduino IDE, lihat angka baud rate di pojok kanan atas jendela monitor.
  3. Samakan nilainya (pilih `115200 baud`).

---

### LCD 1602 I2C Hanya Menampilkan Kotak Hitam / Blank
* **Penyebab:**
  1. Nilai kontras pada trimpot/potensiometer biru di punggung modul I2C belum disetel.
  2. Alamat I2C pada kode salah (`0x27` vs `0x3F`).
  3. Kabel SDA (A4) dan SCL (A5) terbalik.
* **Solusi:**
  1. Ambil obeng kecil minus, putar perlahan potensiometer biru di belakang LCD hingga baris karakter teks terlihat jelas.
  2. Jalankan sketch `i2c_scanner.ino` dari Modul 09 untuk memastikan alamat heksadesimal modul kamu.
  3. Pastikan pin **SDA LCD** terhubung ke **A4 Uno** dan **SCL LCD** terhubung ke **A5 Uno**.

---

### Arduino Hang / Macet Total saat Membaca I2C
* **Penyebab:** Fungsi `Wire.endTransmission()` atau `Wire.requestFrom()` terkunci selamanya jika jalur sinyal SDA atau SCL terputus, short ke ground, atau slave I2C tidak merespon (tidak mengirim bit ACK).
* **Solusi:**
  1. Pastikan modul I2C menerima tegangan VCC 5V yang stabil.
  2. Tambahkan batas waktu (*timeout*) bus I2C bawaan Wire (tersedia sejak Arduino core modern):
     ```cpp
     Wire.begin();
     Wire.setWireTimeout(3000, true); // Timeout 3000 mikrodetik, reset bus otomatis jika macet
     ```

---

## 4. Gangguan Hardware & Komponen Elektrik

### Tombol Ditekan Sekali, tetapi Terbaca Beberapa Kali (Double Trigger)
* **Penyebab:** *Contact Bounce* mekanik. Pelat tembaga di dalam tombol mikro bergetar selama 5–20 milidetik saat ditekan.
* **Solusi:**
  Terapkan logika software debouncing menggunakan `millis()` (Modul 05) atau pasang kapasitor keramik $100\text{nF}$ secara paralel melintasi pin tombol ke Ground.

---

### Nilai Pin Input Melompat-lompat Acak (HIGH dan LOW Sendiri)
* **Penyebab:** Pin dikonfigurasi sebagai `INPUT` biasa tanpa resistor pull-up atau pull-down fisik. Pin berada dalam kondisi *floating* (mengambang) dan menangkap gelombang elektromagnetik sekitar sebagai sinyal.
* **Solusi:**
  Aktifkan pull-up internal ATmega328P pada baris `pinMode`:
  ```cpp
  pinMode(PIN_TOMBOL, INPUT_PULLUP);
  ```
  Tombol dihubungkan antara pin Arduino dan pin **GND** (logika Active-LOW).

---

### Arduino Me-Reset Sendiri Saat Relay Aktif
* **Penyebab:**
  1. **Drop Tegangan (Brownout):** Koil relay membutuhkan arus sekitar 70–90 mA saat aktif. Jika Arduino diberi daya dari port USB yang lemah atau kabel jumper longgar, tegangan 5V turun sesaat di bawah 4.3V sehingga sirkuit Brown-out Detector (BOD) internal mikrokontroler memicu restart otomatis.
  2. **Inductive Kickback (Back EMF):** Medan magnet pada koil relay memicu lonjakan tegangan balik tinggi ke jalur ground saat relay dimatikan.
* **Solusi:**
  1. Pasang kapasitor elektrolit besar ($100\mu\text{F} - 470\mu\text{F}$) di antara jalur 5V dan GND pada breadboard untuk meredam lonjakan beban sesaat.
  2. Jika menggunakan modul relay dengan jumper *JD-VCC*, lepas jumper tersebut dan beri daya koil relay dari sumber adaptor 5V terpisah (Modul 08).
  3. Pastikan modul relay dilengkapi dengan dioda flyback dan optocoupler.

---

### LED Mati Total atau Langsung Rusak (Terbakar)
* **Penyebab:**
  1. Kaki anoda (+) dan katoda (-) terpasang terbalik.
  2. LED dipasang langsung ke pin 5V tanpa resistor pembatas arus ($220\Omega$). Arus berlebih menghancurkan junction semikonduktor dioda seketika.
* **Solusi:**
  1. Periksa kaki LED: kaki yang lebih panjang adalah Anoda (+). Sisi badan LED yang memiliki potongan rata/pipih adalah Katoda (-).
  2. Selalu pasang resistor minimal $220\Omega$ secara seri di salah satu kaki LED.
