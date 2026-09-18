# Modul 06: Pulse Width Modulation (PWM) & Dimmer LED

Pin digital pada mikrokontroler hanya bisa mengeluarkan dua kondisi tegangan: 0V (LOW) atau 5V (HIGH). Mikrokontroler ATmega328P tidak memiliki konverter Digital-to-Analog (DAC) perangkat keras untuk mengeluarkan tegangan murni 2.5V atau 3.1V.

Modul ini membahas teknik **Pulse Width Modulation (PWM)** untuk mensimulasikan tegangan analog rata-rata, pin PWM pada Uno R3, serta cara membuat pengatur kecerahan (dimmer) LED bertingkat menggunakan dua tombol.

---

## 1. Konsep Dasar PWM & Duty Cycle

PWM bekerja dengan cara menyalakan dan mematikan sinyal digital dalam frekuensi yang sangat cepat (ratusan kali per detik). Karena mata manusia memiliki keterbatasan respons visual (*Persistence of Vision*), mata kita tidak melihat kedipan cepat tersebut, melainkan merasakan nilai intensitas rata-ratanya.

Proporsi waktu sinyal berada dalam kondisi `HIGH` terhadap total satu periode gelombang disebut **Duty Cycle**:

$$\text{Duty Cycle (\%)} = \frac{t_{\text{ON}}}{t_{\text{ON}} + t_{\text{OFF}}} \times 100\%$$

```text
Duty Cycle 0% (analogWrite = 0)
0V -------------------------------------------------- Tegangan Rata-rata = 0V

Duty Cycle 25% (analogWrite = 64)
5V +---+             +---+             +---+
0V |   +-------------+   +-------------+   +--------- Tegangan Rata-rata = 1.25V

Duty Cycle 50% (analogWrite = 127)
5V +-------+         +-------+         +-------+
0V |       +---------+       +---------+       +----- Tegangan Rata-rata = 2.5V

Duty Cycle 75% (analogWrite = 191)
5V +-----------+     +-----------+     +-----------+
0V |           +-----+           +-----+           +- Tegangan Rata-rata = 3.75V

Duty Cycle 100% (analogWrite = 255)
5V -------------------------------------------------- Tegangan Rata-rata = 5V
   |<--- Periode --->|
```

Tegangan efektif rata-rata yang diterima oleh LED adalah:

$$V_{\text{rata-rata}} = \frac{\text{Duty Cycle}}{100} \times 5\text{V}$$

---

## 2. Pin PWM pada Arduino Uno R3

Hanya pin digital yang memiliki tanda gelombang tilde (**`~`**) di samping nomornya yang dapat menghasilkan sinyal PWM perangkat keras:

| Pasangan Pin | Timer Hardware | Frekuensi Default |
|---|---|---|
| **Pin D5 & Pin D6** | Timer 0 (8-bit) | $\approx 980\text{ Hz}$ *(Timer ini juga dipakai `millis()`)* |
| **Pin D9 & Pin D10** | Timer 1 (16-bit) | $\approx 490\text{ Hz}$ |
| **Pin D3 & Pin D11** | Timer 2 (8-bit) | $\approx 490\text{ Hz}$ |

Fungsi untuk mengaktifkan sinyal PWM di Arduino adalah:
```cpp
analogWrite(pin, nilai);
```
Parameter `nilai` memiliki resolusi 8-bit, dengan rentang integer dari **`0`** (mati total, 0% duty cycle) sampai **`255`** (menyala penuh, 100% duty cycle).

> [!NOTE]
> Meskipun namanya diawali dengan kata `analog`, fungsi `analogWrite()` **tidak ada hubungannya dengan pin Analog In (A0–A5)**. Fungsi ini hanya berjalan pada pin digital PWM bertanda `~`.

---

## 3. Komponen & Rangkaian Sirkuit

* 1x Arduino Uno R3
* 1x LED 5mm
* 1x Resistor $220\Omega$
* 2x Push Button (Tombol 1: Tambah Kecerahan, Tombol 2: Kurangi Kecerahan)
* Breadboard & kabel jumper

```text
                        RANGKAIAN MODUL 06
                 +-------------------------------+
                 |        ARDUINO UNO R3         |
                 |                               |
                 |    Pin D9 (PWM ~)             |
                 |       |                       |
                 |       v                       |
                 |   [Anoda LED]                 |
                 |       |                       |
                 |    (LED 5mm)                  |
                 |       |                       |
                 |   [Katoda LED]                |
                 |       |                       |
                 |   [Resistor 220 Ohm]          |
                 |       |                       |
                 |    Pin GND                    |
                 |                               |
                 |    Pin D2                     |
                 |       |                       |
                 |       +--- [Button 1: UP] ----+--- GND
                 |                               |
                 |    Pin D3                     |
                 |       |                       |
                 |       +--- [Button 2: DOWN] --+--- GND
                 +-------------------------------+
```

---

## 4. Program Praktik: Dimmer LED 2 Tombol

Program ini mengatur kecerahan LED dalam 10 tingkatan. Tombol pada pin D2 menaikkan kecerahan sebesar 25 poin, dan tombol pada pin D3 menurunkan kecerahan.

```cpp
// led_dimmer_buttons.ino
// Mengatur tingkat kecerahan LED menggunakan 2 tombol (Up & Down)

const uint8_t PIN_LED         = 9;  // Pin PWM ~
const uint8_t PIN_BTN_UP      = 2;  // Tombol Tambah Kecerahan
const uint8_t PIN_BTN_DOWN    = 3;  // Tombol Kurangi Kecerahan

int levelKecerahan = 0;             // Rentang 0 s/d 255
const int LANGKAH_KECERAHAN = 25;   // Nilai kenaikan per klik (~10 tingkat)

// Pelacak status tombol untuk debouncing
int statusUpTerakhir     = HIGH;
int statusDownTerakhir   = HIGH;
uint32_t waktuUpTerakhir   = 0;
uint32_t waktuDownTerakhir = 0;
const uint32_t JEDA_DEBOUNCE = 50;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN_UP, INPUT_PULLUP);
  pinMode(PIN_BTN_DOWN, INPUT_PULLUP);

  // Inisialisasi LED awal mati
  analogWrite(PIN_LED, levelKecerahan);

  // Debugging serial
  Serial.begin(115200);
  Serial.println(F("Dimmer LED Siap. Tekan D2 (Up) atau D3 (Down)."));
}

void loop() {
  uint32_t sekarang = millis();

  // --- 1. Pemrosesan Tombol UP (Pin D2) ---
  int bacaUp = digitalRead(PIN_BTN_UP);
  if (bacaUp != statusUpTerakhir) {
    waktuUpTerakhir = sekarang;
  }
  if ((sekarang - waktuUpTerakhir) > JEDA_DEBOUNCE) {
    // Tombol baru ditekan (transisi HIGH ke LOW)
    static int statusUpStabil = HIGH;
    if (bacaUp != statusUpStabil) {
      statusUpStabil = bacaUp;
      if (statusUpStabil == LOW) {
        levelKecerahan += LANGKAH_KECERAHAN;
        if (levelKecerahan > 255) {
          levelKecerahan = 255; // Batas atas 100%
        }
        analogWrite(PIN_LED, levelKecerahan);
        laporKecerahan();
      }
    }
  }
  statusUpTerakhir = bacaUp;

  // --- 2. Pemrosesan Tombol DOWN (Pin D3) ---
  int bacaDown = digitalRead(PIN_BTN_DOWN);
  if (bacaDown != statusDownTerakhir) {
    waktuDownTerakhir = sekarang;
  }
  if ((sekarang - waktuDownTerakhir) > JEDA_DEBOUNCE) {
    static int statusDownStabil = HIGH;
    if (bacaDown != statusDownStabil) {
      statusDownStabil = bacaDown;
      if (statusDownStabil == LOW) {
        levelKecerahan -= LANGKAH_KECERAHAN;
        if (levelKecerahan < 0) {
          levelKecerahan = 0; // Batas bawah 0%
        }
        analogWrite(PIN_LED, levelKecerahan);
        laporKecerahan();
      }
    }
  }
  statusDownTerakhir = bacaDown;
}

void laporKecerahan() {
  Serial.print(F("Tingkat Kecerahan PWM: "));
  Serial.print(levelKecerahan);
  Serial.print(F(" / 255 ("));
  Serial.print((levelKecerahan * 100) / 255);
  Serial.println(F("%)"));
}
```

### Simulasi Interaktif Wokwi:
* **Tautan Proyek Simulasi**: [Simulasi Wokwi - Modul 06: LED Dimmer PWM](https://wokwi.com/projects/475523980835133441)
* File diagram sirkuit dan kode program tersedia di direktori [code/led_dimmer_buttons/](code/led_dimmer_buttons/).

---

## 5. Ringkasan

1. PWM mengatur proporsi waktu gelombang `HIGH` terhadap `LOW` (Duty Cycle) untuk mensimulasikan variasi tegangan analog rata-rata.
2. Pada Uno R3, PWM tersedia pada pin D3, D5, D6, D9, D10, dan D11 (bertanda `~`).
3. Fungsi `analogWrite(pin, nilai)` menerima rentang angka $0 - 255$ (resolusi 8-bit).
4. Kita dapat mengombinasikan logika debounce tombol non-blocking dengan PWM untuk membuat kontrol antarmuka fisik yang responsif.

---

Pada modul berikutnya, **[Modul 07: Serial UART & Antarmuka I2C LCD 1602](../07_serial_dan_i2c_lcd1602/README.md)**, kita akan menambahkan modul display LCD 1602 via bus I2C (PCF8574) untuk menampilkan informasi teks, angka variabel, dan grafik kustom secara langsung pada layar fisik.
