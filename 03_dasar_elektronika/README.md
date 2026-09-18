# Modul 03: Dasar Elektronika untuk Mikrokontroler

Sebelum menghubungkan komponen fisik ke board Arduino, kita perlu memahami dasar-dasar kelistrikan: cara menghitung resistor untuk LED, cara kerja kontak tombol mekanik, prinsip isolasi pada modul relay, dan batasan aman arus listrik mikrokontroler.

---

## 1. Tiga Besaran Listrik & Hukum Ohm

Sirkuit mikrokontroler bekerja berdasarkan tiga besaran dasar:

| Besaran | Simbol | Satuan | Analogi Aliran Air |
|---|---|---|---|
| **Tegangan (Voltage)** | $V$ | Volt ($\text{V}$) | Tekanan air di dalam pipa |
| **Arus (Current)** | $I$ | Ampere ($\text{A}$) atau miliampere ($\text{mA}$) | Debit atau volume air yang mengalir |
| **Hambatan (Resistance)** | $R$ | Ohm ($\Omega$) | Penyempitan pipa yang menahan laju air |

Ketiganya dihubungkan oleh **Hukum Ohm**:

$$V = I \times R \quad \Longleftrightarrow \quad I = \frac{V}{R} \quad \Longleftrightarrow \quad R = \frac{V}{I}$$

Besaran daya listrik ($P$) diukur dalam satuan Watt:

$$P = V \times I$$

---

## 2. Memahami Komponen Hardware Kit

### 2.1 LED (Light Emitting Diode)
LED adalah komponen semikonduktor yang memancarkan cahaya saat dialiri arus listrik searah. Berbeda dengan resistor, LED memiliki polaritas:
* **Anoda (+)**: Kaki yang lebih panjang. Terhubung ke kutub positif tegangan.
* **Katoda (-)**: Kaki yang lebih pendek. Di dalam badan plastik LED, elektrodanya lebih besar menyerupai bendera, dan tepi plastik di sisi katoda berbentuk pipih. Terhubung ke Ground.

```text
       Anoda (+)             Katoda (-)
       [Kaki Panjang]       [Kaki Pendek / Sisi Pipih]
             |                     |
             +--------|>|----------+
                   LED Simbol
```

Setiap LED memiliki karakteristik **tegangan jatuh maju (Forward Voltage, $V_f$)** dan **arus kerja aman ($I_f$)**:
* LED Merah / Kuning: $V_f \approx 1.8\text{V} - 2.0\text{V}$
* LED Hijau / Biru: $V_f \approx 3.0\text{V} - 3.2\text{V}$
* Arus kerja yang aman untuk LED 5mm standar: $I_f \approx 10\text{mA} - 20\text{mA}$ ($0.010\text{A} - 0.020\text{A}$)

#### Mengapa LED Selalu Membutuhkan Resistor Pembatas Arus?
LED memiliki resistansi internal yang sangat kecil saat menyala. Jika Anda menghubungkan LED langsung dari pin 5V ke GND tanpa resistor, LED akan menarik arus sebesar mungkin dari mikrokontroler hingga melampaui batas fisiknya. Akibatnya, LED akan terbakar seketika dan pin output mikrokontroler berisiko rusak permanen.

#### Menghitung Nilai Resistor LED
Kita menggunakan Hukum Ohm untuk menghitung sisa tegangan yang harus ditahan oleh resistor:

$$R = \frac{V_{\text{sumber}} - V_f}{I_f}$$

Jika kita menggunakan pin Arduino 5V, LED merah ($V_f = 2.0\text{V}$), dan menargetkan arus aman $15\text{mA}$ ($0.015\text{A}$):

$$R = \frac{5\text{V} - 2.0\text{V}}{0.015\text{A}} = \frac{3.0\text{V}}{0.015\text{A}} = 200\Omega$$

Di pasaran, nilai resistor standar terdekat yang umum ditemukan adalah **$220\Omega$** atau **$330\Omega$**.
* Resistor $220\Omega$: Arus mengalir sekitar $13.6\text{mA}$ (cahaya terang, aman).
* Resistor $330\Omega$: Arus mengalir sekitar $9.1\text{mA}$ (cahaya cukup terang, sangat hemat daya).

---

### 2.2 Push Button (Sakelar Taktil)
Push button yang kita gunakan adalah jenis *tactile switch* 4 pin. Meskipun memiliki 4 kaki, di dalamnya kaki-kaki tersebut terhubung berpasangan:

```text
      Pin 1 (A) ----+      +---- Pin 3 (B)
                    |      |
                    o  __  o  <-- Sakelar (Terbuka)
                    |      |
      Pin 2 (A) ----+      +---- Pin 4 (B)
```

* Pin 1 dan Pin 2 saling terhubung langsung secara internal.
* Pin 3 dan Pin 4 saling terhubung langsung secara internal.
* Saat tombol ditekan, jalur A dan jalur B tersambung. Saat dilepas, jalur kembali terputus.

#### Masalah Floating Input (Keadaan Mengambang)
Pin input mikrokontroler memiliki resistansi input yang sangat tinggi (sekitar $100\text{ M}\Omega$). Jika pin digital dikonfigurasi sebagai input dan dihubungkan ke sakelar tanpa resistor referensi:
* Saat tombol ditekan: Pin membaca tegangan pasti (misalnya 5V).
* Saat tombol dilepas: Pin tidak terhubung ke mana pun (*mengambang* / *floating*). Pin ini akan bertindak seperti antena kecil yang menangkap gelombang elektromagnetik dari lingkungan sekitar, menyebabkan nilai pembacaan digital berganti-ganti secara acak antara 0 dan 1.

Untuk mencegah floating, pin wajib diikat ke tegangan pasti menggunakan:
1. **Pull-Up Resistor**: Menarik pin ke 5V saat tombol terbuka, dan ke GND saat tombol ditekan.
2. **Pull-Down Resistor**: Menarik pin ke GND saat tombol terbuka, dan ke 5V saat tombol ditekan.

> Arduino ATmega328P memiliki resistor **Internal Pull-Up** sebesar $20\text{k}\Omega - 50\text{k}\Omega$ di dalam chip. Kita bisa mengaktifkannya langsung lewat baris kode `pinMode(pin, INPUT_PULLUP)` tanpa perlu memasang resistor eksternal tambahan.

---

### 2.3 Modul Relay 5V
Relay adalah sakelar elektromekanik yang digerakkan oleh kumparan magnet. Relay memungkinkan sirkuit kecil bertegangan 5V DC pada Arduino untuk mengendalikan perangkat berdaya besar (misalnya lampu rumah 220V AC atau pompa air DC).

```text
              Sisi Kontrol (Arduino)       Sisi Beban (Tegangan Tinggi)
                                                   +--- NO (Normally Open)
     Pin D8 ---> [Optocoupler] ---> [Koil 5V] ====/
     GND   ---> [  PC817    ]      [Diode D ] ----+--- COM (Common)
                                                  \
                                                   +--- NC (Normally Closed)
```

Komponen pada papan modul relay:
1. **Koil Elektromagnetik (5V)**: Saat dialiri listrik, kumparan menghasilkan medan magnet yang menarik pelat besi kontak sakelar.
2. **Dioda Flyback (Freewheeling Diode)**: Koil adalah beban induktif. Saat arus ke koil diputus mendadak, medan magnet runtuh dan menciptakan lonjakan tegangan balik bertegangan tinggi (*Back EMF*). Dioda flyback menyerap lonjakan ini agar tidak merusak sirkuit kontrol.
3. **Optocoupler (IC PC817)**: Komponen isolasi cahaya. Sinyal dari pin Arduino menyalakan LED infra merah mini di dalam chip, yang kemudian menyalakan phototransistor. Jalur listrik Arduino dan koil relay terpisah secara optik, mencegah interferensi elektromagnetik kembali ke mikrokontroler.

#### Terminal Terminal Beban:
* **COM (Common)**: Terminal pusat yang selalu dihubungkan ke salah satu kabel sumber daya beban.
* **NO (Normally Open)**: Normalnya terputus. Terminal ini baru akan tersambung ke COM saat relay diaktifkan oleh Arduino.
* **NC (Normally Closed)**: Normalnya tersambung. Terminal ini tersambung ke COM saat relay mati, dan terputus saat relay aktif.

---

## 3. Batasan Kelistrikan Pin Arduino Uno R3

Salah satu penyebab paling umum rusaknya chip ATmega328P adalah menghubungkan komponen yang membutuhkan arus terlalu besar langsung ke pin I/O:

| Parameter | Nilai Batas | Keterangan |
|---|---|---|
| **Arus kerja aman per pin GPIO** | **$\le 20\text{mA}$** | Beban LED, optocoupler modul, input sensor |
| **Batas arus absolut per pin GPIO** | **$40\text{mA}$** | Melebihi batas ini merusak jalur silikon pin |
| **Total arus seluruh pin GPIO gabungan** | **$200\text{mA}$** | Batas disipasi daya pin VCC/GND chip |
| **Tegangan input pin GPIO** | **$0\text{V} - 5\text{V}$** | Tegangan $> 5.5\text{V}$ membakar transistor input |

### Mengapa Koil Relay Tidak Boleh Dihubungkan Langsung ke Pin Arduino?
Sebuah koil relay 5V standar membutuhkan arus sekitar **$70\text{mA} - 90\text{mA}$** untuk menarik armatur sakelar. Menghubungkan koil relay secara langsung ke pin GPIO Arduino (yang batas amannya hanya $20\text{mA}$) akan menyebabkan pin kelebihan beban (*overcurrent*) dan hangus.

Itulah alasan kita **wajib menggunakan Modul Relay** yang sudah dilengkapi transistor driver dan optocoupler. Pin Arduino hanya mengirim sinyal kendali kecil (sekitar $2\text{mA}-5\text{mA}$ untuk menyalakan LED di optocoupler), sementara arus besar $80\text{mA}$ diambil langsung dari pin suplai daya 5V.

---

## 4. Cara Menggunakan Breadboard

Breadboard digunakan untuk merangkai komponen tanpa perlu menyolder:

```text
       (+) [ O O O O O ... O O O O O ] <-- Jalur Bus Daya Positif (Horizontal)
       (-) [ O O O O O ... O O O O O ] <-- Jalur Bus Ground (Horizontal)

           A   B   C   D   E
       1  [O   O   O   O   O]  <-- Lubang 1A s/d 1E terhubung vertikal
       2  [O   O   O   O   O]
       3  [O   O   O   O   O]
       ======================= <-- Parit Pemisah IC (Lembah Tengah)
       1  [O   O   O   O   O]  <-- Lubang 1F s/d 1J terhubung vertikal
       2  [O   O   O   O   O]
       3  [O   O   O   O   O]
           F   G   H   I   J

       (+) [ O O O O O ... O O O O O ]
       (-) [ O O O O O ... O O O O O ]
```

1. **Jalur Rel Daya (Power Rails)**: Garis horizontal panjang di bagian atas dan bawah bertanda merah $(+)$ dan biru $(-)$. Seluruh lubang di sepanjang baris tersebut terhubung secara horizontal. Biasanya dihubungkan ke pin 5V dan GND Arduino.
2. **Jalur Terminal Komponen**: Lubang bernomor 1 sampai 30/60 terhubung secara vertikal per 5 lubang (kolom A-B-C-D-E terhubung satu sama lain).
3. **Parit Tengah (Center Divider)**: Memisahkan kolom A-E dari kolom F-J. Parit ini dirancang dengan lebar pas untuk menancapkan chip IC (seperti ATmega atau optocoupler) sehingga pin di sisi kiri dan kanan tidak saling korslet.

---

## 5. Ringkasan

1. Hukum Ohm ($V = I \times R$) menentukan hubungan antara tegangan sumber, arus kerja, dan nilai hambatan yang dibutuhkan.
2. LED wajib dipasangi resistor pembatas arus ($220\Omega - 330\Omega$ untuk tegangan 5V) agar arus tidak melebihi $20\text{mA}$.
3. Pin input digital yang dibiarkan tanpa resistor referensi akan berstatus *floating* dan membaca data acak.
4. Pin GPIO Arduino Uno aman mengeluarkan arus maksimal $20\text{mA}$. Komponen berdaya lebih tinggi (seperti koil relay atau motor) harus dikendalikan menggunakan transistor atau modul perantara.

---

Pada modul berikutnya, **[Modul 04: Fondasi Pemrograman Embedded C/C++ pada Arduino](../04_pemrograman_embedded_cpp/README.md)**, kita akan masuk ke penulisan kode: memahami siklus kerja `setup()` dan `loop()`, efisiensi memori, serta alasan mengapa penggunaan objek `String` dilarang keras pada mikrokontroler dengan RAM 2KB.
