# Glosarium Istilah Arduino & Embedded Systems

Daftar istilah penting seputar elektronika, perangkat keras mikrokontroler, dan pemrograman embedded yang digunakan di seluruh modul tutorial ini, disusun menurut abjad.

---

### Active-HIGH / Active-LOW
Logika pengendali pada rangkaian digital:
* **Active-HIGH**: Perangkat aktif/bekerja saat diberi tegangan tinggi (5V / logika `HIGH` / `1`).
* **Active-LOW**: Perangkat aktif/bekerja saat dihubungkan ke ground (0V / logika `LOW` / `0`). Mayoritas modul relay dan tombol dengan pull-up internal bekerja dengan Active-LOW.

### ADC (Analog-to-Digital Converter)
Rangkaian internal mikrokontroler yang bertugas mengubah sinyal tegangan analog kontinu (rentang $0 - 5\text{V}$) menjadi angka integer digital diskrit. Pada ATmega328P (Arduino Uno), resolusi ADC adalah 10-bit ($2^{10} = 1024$ level, yaitu nilai desimal $0$ sampai $1023$).

### Analog In (Pin A0–A5)
Pin khusus pada Arduino Uno yang terhubung ke konverter ADC internal. Dapat digunakan untuk membaca sensor analog (seperti potensiometer atau sensor cahaya) atau difungsikan sebagai pin digital biasa (D14 sampai D19).

### `analogRead()`
Fungsi bawaan untuk membaca nilai tegangan dari pin Analog In. Menghasilkan nilai integer antara $0$ (mewakili 0V) hingga $1023$ (mewakili 5V).

### `analogWrite()`
Fungsi bawaan untuk menghasilkan sinyal PWM (Pulse Width Modulation) pada pin digital bertanda gelombang `~`. Nilai duty cycle diatur dari $0$ (mati total) hingga $255$ (aktif penuh). Fungsi ini tidak menghasilkan tegangan analog murni, melainkan pulsa switching on-off cepat.

### Anoda & Katoda
Dua kutub elektroda pada komponen semikonduktor dioda dan LED:
* **Anoda (+)**: Kaki kutub positif (biasanya berukuran lebih panjang).
* **Katoda (-)**: Kaki kutub negatif yang terhubung ke Ground (biasanya berukuran lebih pendek dengan sisi badan plastik yang pipih).

### ATmega328P
Chip mikrokontroler 8-bit arsitektur AVR buatan Atmel (Microchip) yang menjadi otak pemroses utama papan Arduino Uno R3 dan Nano.

### Auto-Reset (DTR)
Sirkuit pada Arduino yang menghubungkan pin serial DTR (*Data Terminal Ready*) ke pin RESET mikrokontroler via kapasitor $100\text{nF}$. Memungkinkan komputer mereset Arduino secara otomatis saat hendak mengunggah kode baru tanpa perlu menekan tombol reset fisik.

### `avrdude`
Perangkat lunak berbasis command-line (*AVR Downloader/Uploader*) yang digunakan di balik layar oleh Arduino IDE untuk mengirimkan file binary hasil kompilasi (`.hex`) ke memori flash mikrokontroler melalui bootloader.

### Back EMF (Flyback Voltage)
Lonjakan tegangan balik berdaya tinggi yang timbul saat arus listrik pada beban induktif (seperti kumparan/koil relay atau motor) diputus secara mendadak. Dicegah menggunakan Dioda Flyback agar tidak merusak transistor atau pin mikrokontroler.

### Baud Rate
Satuan kecepatan transmisi simbol komunikasi serial per detik (misal `9600` atau `115200` baud). Kecepatan pada baris `Serial.begin()` wajib sama persis dengan angka yang dipilih pada Serial Monitor komputer.

### Blocking vs Non-Blocking
Dua filosofi eksekusi kode:
* **Blocking**: Program menghentikan seluruh pekerjaan lain untuk menunggu suatu kejadian (contoh: `delay(1000)`). Selama masa tunggu, tombol dan sensor tidak dapat dibaca.
* **Non-Blocking**: Program mencatat waktu menggunakan `millis()` dan terus mengeksekusi tugas-tugas lain secara paralel tanpa pernah mengunci jalannya siklus CPU.

### Bootloader
Program kecil yang tersimpan permanen di area teratas memori Flash mikrokontroler. Bertugas mendengarkan koneksi serial USB selama 1 detik setelah reset untuk menerima program baru. Menghilangkan kebutuhan alat flash eksternal (*programmer*).

### Breadboard
Papan plastik berlubang yang digunakan untuk merangkai sirkuit elektronika prototipe tanpa perlu menyolder. Lubang-lubang dihubungkan oleh pelat penjepit logam di bagian dalamnya.

### CH340
Chip terintegrasi (IC) konverter USB-to-UART murah buatan WCH yang umum digunakan pada papan Arduino Uno dan Nano versi clone/kompatibel.

### Contact Bounce (Bouncing)
Fenomena getaran mekanik mikroskopis saat pelat logam sakelar atau tombol pertama kali bersentuhan. Menimbulkan pulsa listrik on-off yang tidak stabil selama $5 - 20\text{ milidetik}$ sebelum benar-benar menempel rapat.

### Debouncing
Metode untuk menyaring dan mengabaikan getaran kontak sakelar mekanik agar penekanan tombol hanya dihitung satu kali. Dapat diselesaikan via software (timer `millis()`) atau hardware (filter low-pass RC).

### Decoupling Capacitor
Kapasitor kecil (biasanya keramik $0.1\mu\text{F}$ atau $100\text{nF}$) yang diletakkan sedekat mungkin dengan pin daya IC untuk menyerap noise listrik frekuensi tinggi dan menjaga kestabilan tegangan catu daya.

### `delay()`
Fungsi untuk menghentikan eksekusi program selama jumlah milidetik tertentu. Termasuk fungsi *blocking* yang dihindari pada arsitektur sistem responsif atau industri.

### Digital I/O
Pin yang hanya dapat membaca atau mengeluarkan dua tingkatan tegangan biner: logika `HIGH` (mendekati 5V) atau logika `LOW` (mendekati 0V/Ground).

### `digitalRead()` & `digitalWrite()`
* `digitalRead(pin)`: Membaca status logika pin digital (mengembalikan `HIGH` atau `LOW`).
* `digitalWrite(pin, nilai)`: Mengatur output pin digital ke tegangan 5V (`HIGH`) atau 0V (`LOW`).

### Duty Cycle
Persentase lamanya sinyal berada dalam kondisi `HIGH` dibandingkan total satu siklus gelombang periodik pada sinyal PWM. Duty cycle 50% berarti sinyal menyala setengah periode dan padam setengah periode.

### EEPROM (Electrically Erasable Programmable Read-Only Memory)
Memori non-volatile internal pada mikrokontroler (1 KB pada ATmega328P) yang datanya tidak hilang saat daya dimatikan. Memiliki batas ketahanan sekitar 100.000 siklus penulisan.

### FSM (Finite State Machine)
Model perancangan logika perangkat lunak di mana sistem diorganisasikan ke dalam beberapa status (*state*) diskrit yang jelas. Sistem hanya berpindah status jika ada pemicu (*event*) yang terdefinisi dengan aturan baku.

### Flash Memory
Memori non-volatile utama berkapasitas 32 KB pada Uno R3 yang berfungsi menyimpan instruksi kode program hasil kompilasi.

### Floating State (Input Mengambang)
Kondisi di mana pin input mikrokontroler tidak terhubung ke tegangan pasti (5V maupun Ground), misalnya tombol yang dibiarkan tanpa resistor pull-up/pull-down. Menyebabkan pin bertindak sebagai antena yang membaca nilai acak 0 dan 1 akibat interferensi elektromagnetik.

### Flyback Diode (Freewheeling Diode)
Dioda penyearah yang dipasang berlawanan polaritas melintasi kumparan induktif (seperti koil relay) untuk membuang lonjakan tegangan balik (*Back EMF*) ke jalur aman.

### Ground (GND)
Titik referensi nol volt ($0\text{V}$) dalam rangkaian listrik. Seluruh arus listrik yang mengalir dari sumber tegangan positif pada akhirnya harus kembali ke Ground untuk membentuk rangkaian tertutup (*closed loop*).

### Harvard Architecture
Arsitektur komputer di mana jalur bus data dan jalur bus instruksi program dipisahkan secara fisik. Diterapkan pada mikrokontroler AVR (memori Flash dan SRAM memiliki bus memori masing-masing).

### Heap & Stack
Dua area alokasi dinamis di dalam memori SRAM:
* **Stack**: Menyimpan variabel lokal fungsi, parameter, dan alamat kembali pemanggilan fungsi secara terstruktur LIFO (*Last In, First Out*).
* **Heap**: Tempat alokasi memori dinamis (misalnya saat menggunakan kelas `String` atau `malloc()`). Penggunaan heap pada RAM kecil berisiko menimbulkan tabrakan memori (*stack collision*).

### I2C (Inter-Integrated Circuit)
Protokol komunikasi serial sinkron dua kabel (*Two-Wire Interface / TWI*) berbasis Master-Slave. Hanya membutuhkan dua pin: **SDA** (data) dan **SCL** (clock). Memungkinkan puluhan sensor dan modul terhubung paralel pada kabel yang sama melalui pengalamatan biner 7-bit.

### ICSP (In-Circuit Serial Programming)
Konektor header 6-pin pada papan Arduino untuk memprogram mikrokontroler secara langsung menggunakan hardware programmer eksternal (SPI) tanpa perantara USB bootloader.

### `INPUT_PULLUP`
Mode konfigurasi pin pada `pinMode()` yang menghubungkan resistor pull-up internal ($20\text{k}\Omega - 50\text{k}\Omega$) mikrokontroler ke jalur 5V. Menyederhanakan pemasangan tombol ke ground tanpa resistor luar.

### Interrupt (Interupsi) & ISR
Mekanisme perangkat keras di mana peristiwa luar (misal penekanan tombol di pin D2) seketika menghentikan eksekusi kode utama untuk menjalankan fungsi penanganan khusus (**Interrupt Service Routine / ISR**) dalam hitungan mikrodetik.

### LCD 1602
Layar liquid crystal display karakter yang mampu menampilkan teks sebanyak 16 kolom dan 2 baris (total 32 karakter). Umumnya dipasangi backpack modul PCF8574 agar dapat dikendalikan via protokol I2C.

### LED (Light Emitting Diode)
Komponen semikonduktor yang memancarkan cahaya saat dialiri arus searah dari anoda ke katoda. Wajib dipasangi resistor pembatas arus ($220\Omega - 330\Omega$) agar arusnya tidak melebihi batas aman $20\text{mA}$.

### Macro `F()`
Sintaks Arduino (`F("teks")`) yang memaksa string literal konstan tetap disimpan di memori Flash ROM dan tidak disalin ke memori kerja SRAM saat booting. Kunci utama menghemat RAM pada program dengan banyak teks Serial/LCD.

### `millis()` & `micros()`
Fungsi pencatat waktu yang mengembalikan jumlah milidetik (atau mikrodetik) yang telah berlalu sejak papan Arduino pertama kali dinyalakan. Digunakan sebagai basis penjadwal waktu non-blocking. Nilai akan meluap (*rollover*) kembali ke 0 setelah sekitar 50 hari.

### Optocoupler (Optoisolator)
Komponen isolasi listrik yang memindahkan sinyal kendali menggunakan cahaya inframerah. Memisahkan jalur ground sirkuit mikrokontroler dari sirkuit beban relay untuk mencegah lonjakan tegangan merusak mikroprosesor.

### PCF8574
IC expander I/O 8-bit yang mengubah komunikasi serial I2C (SDA/SCL) menjadi 8 jalur pin paralel, umum digunakan sebagai modul backpack pada LCD 1602.

### `pinMode()`
Fungsi inisialisasi di `setup()` untuk menentukan peran pin digital: sebagai masukan (`INPUT`), masukan berhambatan dalam (`INPUT_PULLUP`), atau keluaran daya (`OUTPUT`).

### Polyfuse
Sekering otomatis pada sirkuit USB Arduino Uno yang memutus aliran listrik jika arus melebihi $500\text{mA}$ (misalnya akibat korsleting) dan otomatis tersambung kembali begitu suhu sekering mendingin.

### Pull-Up & Pull-Down Resistor
Resistor bernilai besar ($10\text{k}\Omega - 50\text{k}\Omega$) yang dipasang pada pin input untuk memastikan pin berada pada level tegangan pasti saat sakelar terbuka:
* **Pull-Up**: Mengikat pin ke 5V (default `HIGH`).
* **Pull-Down**: Mengikat pin ke 0V/GND (default `LOW`).

### PWM (Pulse Width Modulation)
Teknik modulasi lebar pulsa digital untuk mensimulasikan nilai tegangan analog rata-rata dengan cara menyalakan dan mematikan sinyal dengan kecepatan frekuensi ratusan Hertz.

### Relay
Sakelar elektromekanik yang digerakkan oleh kumparan elektromagnetik. Memungkinkan mikrokontroler 5V berdaya rendah menyalakan beban tegangan dan arus tinggi (seperti lampu 220V AC) secara aman.

### SCL & SDA
Dua jalur kabel komunikasi bus I2C:
* **SCL (Serial Clock)**: Pin detak sinkronisasi pengiriman bit (Pin A5 pada Uno R3).
* **SDA (Serial Data)**: Pin transmisi data biner dua arah (Pin A4 pada Uno R3).

### Serial Monitor & Serial Plotter
Alat diagnostik terintegrasi pada Arduino IDE:
* **Serial Monitor**: Terminal teks untuk mengirim dan membaca data komunikasi serial UART.
* **Serial Plotter**: Grafik kurva visual real-time yang memplot data angka serial secara dinamis.

### `setup()` & `loop()`
Dua blok fungsi wajib pada program Arduino:
* `setup()`: Tempat inisialisasi hardware, pin, dan komunikasi serial; dieksekusi satu kali di awal.
* `loop()`: Fungsi yang dipanggil berulang kali tanpa henti selama mikrokontroler hidup.

### SPI (Serial Peripheral Interface)
Protokol komunikasi serial sinkron 4 kabel berkecepatan tinggi (MOSI, MISO, SCK, CS/SS) yang umum digunakan untuk modul SD Card, RFID RC522, dan sensor berkecepatan megahertz.

### SRAM (Static RAM)
Memori kerja sementara berkapasitas 2 KB (2.048 byte) pada ATmega328P untuk menyimpan variabel aktif runtime. Data hilang saat papan kehilangan daya (*volatile*).

### `String` vs `char[]`
* **`String`**: Objek C++ dinamis dengan alokasi heap. Berbahaya digunakan pada chip AVR RAM 2KB karena memicu fragmentasi memori (*heap fragmentation*) yang berujung crash acak.
* **`char[]`**: Array karakter C murni dengan ukuran memori statis tetap. Sangat aman dan direkomendasikan untuk seluruh aplikasi mikrokontroler.

### UART (Universal Asynchronous Receiver-Transmitter)
Modul periferal komunikasi serial asinkron perangkat keras pada mikrokontroler yang bekerja melalui dua pin: **TX** (Transmit / kirim) dan **RX** (Receive / terima). Pada Uno R3, UART terhubung ke pin D0 dan D1 serta port USB.

### `volatile`
Kata kunci (*type qualifier*) pada deklarasi variabel C/C++ yang menginstruksikan compiler untuk selalu membaca nilai variabel langsung dari RAM dan bukan dari register CPU cache. **Wajib digunakan** pada setiap variabel yang diubah di dalam fungsi ISR interupsi perangkat keras.

### Voltage Divider (Pembagi Tegangan)
Rangkaian sederhana dua resistor seri yang membagi tegangan input menjadi pecahan proporsional yang lebih kecil:

$$V_{\text{out}} = V_{\text{in}} \times \frac{R_2}{R_1 + R_2}$$

Digunakan untuk membaca sensor resistif (seperti LDR) dan menurunkan level logika 5V ke 3.3V.

### Wokwi
Platform simulator sirkuit dan perangkat lunak mikrokontroler berbasis web yang mendukung simulasi penuh Arduino Uno R3, ESP32, LCD I2C, tombol, relay, dan komponen elektronika tanpa membutuhkan perangkat fisik.
