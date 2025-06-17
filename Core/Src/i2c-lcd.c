/**
 * File: i2c-lcd.c (RTOS-aware version)
 * Deskripsi: Library LCD ini telah dimodifikasi untuk bekerja dengan
 * RTOS (FreeRTOS/CMSIS-OS) dengan mengganti HAL_Delay()
 * menjadi osDelay().
 *
 * Diunggah oleh pengguna dan diperbaiki.
 */

#include "i2c-lcd.h"
#include "cmsis_os.h" // WAJIB: Sertakan header CMSIS-OS untuk osDelay()

// PENTING: Pastikan handle I2C ini sesuai dengan yang Anda gunakan di CubeMX.
// File main.c sebelumnya menggunakan hi2c1, file ini menggunakan hi2c2.
// Ganti 'hi2c2' di bawah ini jika perlu.
extern I2C_HandleTypeDef hi2c2;

// PENTING: Pastikan alamat I2C ini benar untuk modul LCD Anda.
// Alamat umum adalah 0x4E atau 0x7E (setara 0x27 << 1).
#define SLAVE_ADDRESS_LCD 0x4E

void lcd_send_cmd(char cmd)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (cmd & 0xf0);
	data_l = ((cmd << 4) & 0xf0);
	data_t[0] = data_u | 0x0C; // en=1, rs=0 -> bxxxx1100
	data_t[1] = data_u | 0x08; // en=0, rs=0 -> bxxxx1000
	data_t[2] = data_l | 0x0C; // en=1, rs=0 -> bxxxx1100
	data_t[3] = data_l | 0x08; // en=0, rs=0 -> bxxxx1000
	HAL_I2C_Master_Transmit(&hi2c2, SLAVE_ADDRESS_LCD, (uint8_t *)data_t, 4, 100);
}

void lcd_send_data(char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data & 0xf0);
	data_l = ((data << 4) & 0xf0);
	data_t[0] = data_u | 0x0D; // en=1, rs=1 -> bxxxx1101
	data_t[1] = data_u | 0x09; // en=0, rs=1 -> bxxxx1001
	data_t[2] = data_l | 0x0D; // en=1, rs=1 -> bxxxx1101
	data_t[3] = data_l | 0x09; // en=0, rs=1 -> bxxxx1001
	HAL_I2C_Master_Transmit(&hi2c2, SLAVE_ADDRESS_LCD, (uint8_t *)data_t, 4, 100);
}

void lcd_clear(void)
{
	lcd_send_cmd(0x01); // Perintah untuk clear display
	osDelay(2);			// Clear display butuh waktu lebih lama
}

void lcd_put_cur(int row, int col)
{
	switch (row)
	{
	case 0:
		col |= 0x80;
		break;
	case 1:
		col |= 0xC0;
		break;
	}
	lcd_send_cmd(col);
}

void lcd_init(void)
{
	// 4 bit initialisation
	osDelay(50); // Menggunakan osDelay, bukan HAL_Delay
	lcd_send_cmd(0x30);
	osDelay(5); // Menggunakan osDelay
	lcd_send_cmd(0x30);
	osDelay(1); // Menggunakan osDelay
	lcd_send_cmd(0x30);
	osDelay(10);		// Menggunakan osDelay
	lcd_send_cmd(0x20); // 4bit mode
	osDelay(10);		// Menggunakan osDelay

	// dislay initialisation
	lcd_send_cmd(0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	osDelay(1);
	lcd_send_cmd(0x08); // Display on/off control --> D=0,C=0, B=0  ---> display off
	osDelay(1);
	lcd_send_cmd(0x01); // clear display
	osDelay(2);			// Perintah clear butuh waktu
	lcd_send_cmd(0x06); // Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	osDelay(1);
	lcd_send_cmd(0x0C); // Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
}

void lcd_send_string(char *str)
{
	while (*str)
		lcd_send_data(*str++);
}
