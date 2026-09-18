# Cheatsheet Arduino Uno R3 & Embedded C++

Referensi cepat satu halaman untuk konfigurasi pin, batasan elektrik, manipulasi bitwise register AVR, dan sintaks penting pemrograman Arduino.

---

## 1. Spesifikasi Teknis & Batas Elektrik Uno R3

| Parameter | Nilai Aman (Operating) | Batas Maksimum Absolut (Bahaya) |
|---|---|---|
| **Tegangan Kerja Chip (VCC)** | $5.0\text{V}$ | $5.5\text{V}$ |
| **Tegangan Input Jack DC / Vin** | $7.0\text{V} - 12.0\text{V}$ | $6.0\text{V} - 20.0\text{V}$ |
| **Arus Output per Pin I/O** | $\le 20\text{ mA}$ | $40\text{ mA}$ (merusak pin jika terlampaui) |
| **Arus Total Seluruh Pin VCC / GND** | $\le 150\text{ mA}$ | $200\text{ mA}$ |
| **Arus Pin 3.3V Internal** | $\le 50\text{ mA}$ | $150\text{ mA}$ |
| **Kecepatan Clock CPU** | $16\text{ MHz}$ (Kristal Eksternal) | - |
| **Kapasitas Memori Flash** | $32\text{ KB}$ (0.5 KB dipakai bootloader) | - |
| **Kapasitas SRAM** | $2\text{ KB}$ ($2.048\text{ byte}$) | - |
| **Kapasitas EEPROM** | $1\text{ KB}$ ($1.024\text{ byte}$) | - |

---

## 2. Pemetaan Pin Kritis (Pinout Matrix)

| Kategori Pin | Label Pin di Board | Fitur Khusus & Peripheral Internal |
|---|---|---|
| **UART Serial (Hardware)** | `0 (RX)`, `1 (TX)` | Terhubung ke USB chip. Hindari memakai pin ini untuk input/output sirkuit! |
| **Hardware External Interrupt** | `2 (INT0)`, `3 (INT1)` | Mendukung trigger `LOW`, `CHANGE`, `FALLING`, `RISING` |
| **PWM (Pulse Width Modulation)** | `3, 5, 6, 9, 10, 11` | Frekuensi pin 5 & 6: $980\text{ Hz}$. Pin 3, 9, 10, 11: $490\text{ Hz}$ |
| **SPI Bus** | `10 (SS)`, `11 (MOSI)`, `12 (MISO)`, `13 (SCK)` | Komunikasi serial sinkron kecepatan tinggi |
| **Onboard Built-in LED** | `13 (LED_BUILTIN)` | Terhubung ke LED kecil di board via op-amp |
| **I2C Bus (Two-Wire)** | `A4 (SDA)`, `A5 (SCL)` | Wajib resistor pull-up ($4.7\text{k}\Omega$) ke 5V (modul biasanya sudah ada) |
| **Analog In (10-bit ADC)** | `A0 - A5` | Resolusi: 0–1023 ($4.88\text{ mV}$ per unit step). Bisa dipakai sebagai pin digital D14–D19 |

---

## 3. Manipulasi Register Bitwise (Port I/O)

Uno R3 menggunakan chip ATmega328P dengan 3 kelompok port 8-bit:
* **PORTB**: Pin Digital D8 sampai D13 (PB0 s.d PB5)
* **PORTC**: Pin Analog A0 sampai A5 (PC0 s.d PC5)
* **PORTD**: Pin Digital D0 sampai D7 (PD0 s.d PD7)

Setiap port memiliki 3 register:
1. `DDRx` (*Data Direction Register*): `1` = OUTPUT, `0` = INPUT
2. `PORTx` (*Data Register*): Output `HIGH`/`LOW` (atau aktifkan pull-up jika pin diset INPUT)
3. `PINx` (*Input Pins Address*): Membaca status level logika pin saat ini

### Operasi Bitwise Penting:
```cpp
// 1. Set pin D13 (PB5) sebagai OUTPUT tanpa mengubah pin lain:
DDRB |= (1 << PB5);

// 2. Set pin D8 (PB0) sebagai INPUT dengan PULL-UP:
DDRB &= ~(1 << PB0);   // DDRB bit 0 = 0 (INPUT)
PORTB |= (1 << PB0);   // PORTB bit 0 = 1 (PULL-UP aktif)

// 3. Menyalakan pin D13 (HIGH) dalam 1 siklus CPU (62.5 nanodetik):
PORTB |= (1 << PB5);

// 4. Mematikan pin D13 (LOW):
PORTB &= ~(1 << PB5);

// 5. Toggle (membalik logika pin D13):
PINB |= (1 << PB5);    // Menulis 1 ke PIN register membalik statusnya secara hardware

// 6. Membaca logika pin D8 (apakah bernilai LOW?):
if (!(PINB & (1 << PB0))) {
    // Tombol ditekan (Active-LOW)
}
```

---

## 4. Rumus Penting Elektronika

### Hukum Ohm
$$V = I \times R \quad \Longleftrightarrow \quad I = \frac{V}{R} \quad \Longleftrightarrow \quad R = \frac{V}{I}$$

### Resistor Pembatas Arus LED
$$R = \frac{V_{\text{sumber}} - V_{\text{LED}}}{I_{\text{LED}}}$$
* Untuk $V_{\text{sumber}} = 5.0\text{V}$, $V_{\text{LED merah}} \approx 2.0\text{V}$, dan target $I = 15\text{ mA}$ ($0.015\text{ A}$):
  $$R = \frac{5.0 - 2.0}{0.015} = \frac{3.0}{0.015} = 200\Omega \quad \longrightarrow \text{Gunakan } 220\Omega \text{ (nilai standar)}$$

---

## 5. Pola Kode Wajib Ingat

### Non-Blocking Timer Pattern (`millis()`)
```cpp
static uint32_t lastTick = 0;
const uint32_t INTERVAL = 500; // 500 ms

if (millis() - lastTick >= INTERVAL) {
    lastTick = millis();
    // Kerjakan tugas periodik di sini tanpa mengunci CPU!
}
```

### Save String to Flash (Makro `F()`)
```cpp
// Mencegah string statis menyita RAM 2KB:
Serial.print(F("Status Sistem: Normal"));
```

### Attach External Interrupt (Pin D2)
```cpp
void setup() {
    pinMode(2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(2), handleEmergencyStop, FALLING);
}

// Variabel yang diubah di dalam ISR WAJIB diberi keyword 'volatile'
volatile bool emergencyTriggered = false;

void handleEmergencyStop() {
    emergencyTriggered = true;
}
```

### Alamat Default I2C Umum
* `0x27` atau `0x3F`: Modul LCD 1602 (PCF8574 / PCF8574A backpack)
* `0x68`: RTC DS1307 / DS3231 atau Sensor Gyro MPU6050
* `0x3C`: Layar OLED 0.96 inch SSD1306
* `0x50` - `0x57`: EEPROM Eksternal 24C32

---

## 6. Referensi Frekuensi Nada Audio (Buzzer Pasif)

| Nada | Frekuensi (Hz) | Nada | Frekuensi (Hz) |
|:---:|:---:|:---:|:---:|
| **C4 (Do)** | $262\text{ Hz}$ | **G4 (Sol)** | $392\text{ Hz}$ |
| **D4 (Re)** | $294\text{ Hz}$ | **A4 (La)**  | $440\text{ Hz}$ |
| **E4 (Mi)** | $330\text{ Hz}$ | **B4 (Si)**  | $494\text{ Hz}$ |
| **F4 (Fa)** | $349\text{ Hz}$ | **C5 (Do Tinggi)** | $523\text{ Hz}$ |

Sintaks: `tone(pin, frekuensi, durasi_ms);` dan `noTone(pin);`

---

## 7. Skema Matriks Keypad 4x4 (16 Tombol / 8 Pin)

| Baris / Kolom | **Col 1 (D5)** | **Col 2 (D4)** | **Col 3 (D3)** | **Col 4 (D2)** |
|:---:|:---:|:---:|:---:|:---:|
| **Row 1 (D9)** | `1` | `2` | `3` | `A` |
| **Row 2 (D8)** | `4` | `5` | `6` | `B` |
| **Row 3 (D7)** | `7` | `8` | `9` | `C` |
| **Row 4 (D6)** | `*` | `0` | `#` | `D` |

Metode: Set 1 Baris ke `LOW` (`OUTPUT`), baca Kolom dengan `INPUT_PULLUP`.

