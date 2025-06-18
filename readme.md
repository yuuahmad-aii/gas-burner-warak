# Dokumentasi Alat: Gas Burner Warak

## Deskripsi Singkat
Alat ini adalah sistem pengendali pembakar gas otomatis berbasis mikrokontroler STM32 (STM32F103C8T6) yang dilengkapi dengan sensor suhu, pemantik otomatis, solenoid gas, dan tampilan LCD I2C. Sistem ini menggunakan FreeRTOS untuk multitasking, sehingga pembacaan suhu, kontrol aktuator, dan update tampilan LCD berjalan secara paralel.

## Fitur Utama
- **Kontrol Suhu Otomatis:** Menjaga suhu sesuai setpoint dengan histeresis untuk mencegah switching berlebihan.
- **Pengapian Otomatis:** Pemantik menyala sebelum solenoid gas dibuka, dengan overlap waktu sesuai parameter.
- **Tampilan LCD:** Menampilkan suhu aktual, setpoint, dan status sistem secara real-time.
- **Keamanan:** Sistem masuk ke mode error jika terjadi kegagalan.
- **Multitasking:** Menggunakan FreeRTOS untuk task pembacaan suhu, kontrol, dan update LCD.

## Blok Diagram Sistem
```
[SENSOR SUHU] --(SPI)--> [STM32] <--(I2C)--> [LCD]
                                 |
                        [PEMANTIK] [SOLENOID GAS]
                                 |
                             [TOMBOL]
```

## Komponen Utama
- **Mikrokontroler:** STM32F103C8T6
- **Sensor Suhu:** MAX6675 (SPI)
- **LCD:** LCD 16x2 I2C
- **Aktuator:** Pemantik (Igniter), Solenoid Gas
- **Tombol:** User Button, Emergency Button, dsb.

## Cara Kerja Sistem
1. **Inisialisasi:** Semua periferal diinisialisasi, LCD menampilkan pesan selamat datang.
2. **Pembacaan Suhu:** Task `ReadTempTask` membaca suhu dari sensor MAX6675 via SPI secara periodik.
3. **Kontrol Aktuator:** Task `ControlTask` mengatur pemantik dan solenoid berdasarkan suhu dan status sistem:
   - Jika suhu di bawah setpoint-histeresis, sistem mulai proses pengapian.
   - Pemantik menyala lebih dulu, lalu solenoid gas dibuka setelah jeda tertentu.
   - Jika suhu sudah mencapai setpoint, aktuator dimatikan.
4. **Update LCD:** Task `UpdateLCDTask` menampilkan suhu, setpoint, dan status sistem di LCD.
5. **Keamanan:** Jika terjadi error (misal sensor tidak terbaca), sistem masuk mode error.

## Parameter yang Dapat Disesuaikan
- `SET_POINT_TEMP` : Setpoint suhu target (default: 80°C)
- `HYSTERESIS` : Histeresis suhu (default: 5°C)
- `IGNITION_TIME_MS` : Waktu pemantik menyala sebelum gas (default: 3000 ms)
- `IGNITION_OVERLAP_MS` : Waktu pemantik & gas menyala bersamaan (default: 1000 ms)

## Status Sistem
- **IDLE:** Sistem menunggu suhu turun di bawah setpoint-histeresis.
- **IGNITING:** Pemantik menyala, lalu solenoid gas dibuka.
- **HEATING:** Gas dan api menyala, sistem memanaskan hingga suhu tercapai.
- **ERROR:** Terjadi kesalahan, aktuator dimatikan.

## Penjelasan Pin GPIO
| Nama Pin      | Fungsi           | Port/Pin      |
|---------------|------------------|---------------|
| USER_BTN      | Tombol User      | GPIOA, PIN 0  |
| BTN_HIJAU     | Tombol Hijau     | GPIOA, PIN 1  |
| BTN_HITAM     | Tombol Hitam     | GPIOA, PIN 2  |
| E_BTN         | Emergency Button | GPIOA, PIN 3  |
| USER_LED      | LED Status       | GPIOB, PIN 2  |
| MAX_CS        | CS MAX6675       | GPIOA, PIN 15 |
| O_PEMANTIK    | Output Pemantik  | GPIOB, PIN 6  |
| O_SELENOID    | Output Solenoid  | GPIOB, PIN 7  |

## Catatan Penggunaan
- Pastikan wiring sensor, aktuator, dan LCD sesuai dengan konfigurasi pin di atas.
- Parameter dapat diubah di file `main.c` pada bagian define.
- Untuk menambah fitur atau mengubah logika, modifikasi pada masing-masing task di `main.c`.
