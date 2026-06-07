#ifndef OLED_H
#define OLED_H

#include <stdint.h>

/* SSD1306 128x32 OLED (0.91 inch) - I2C uzerinden.
 * Altta i2c_b0.c calisir (P1.6 SCL, P1.7 SDA).
 *
 * I2C adresi: oled.c icindeki OLED_I2C_ADDR (varsayilan 0x3C).
 * Bazi modulerinde 0x3D'tir; calistirinca ekran bos kalirsa
 * adresi degistir.
 *
 * 5x7 font, her karakter 6 piksel genislikte (5 + 1 bosluk).
 * Bir page = 8 piksel yukseklik. 128x32 ekran = 4 page (0..3).
 */

void oled_init(void);
void oled_clear(void);

/* Imleci ayarla, sonraki print'ler buradan baslar. */
void oled_set_cursor(uint8_t col, uint8_t page);

void oled_print_char(char c);
void oled_print_str(const char *s);
void oled_print_num(uint16_t n);    /* isaretsiz */
void oled_print_int(int16_t  n);    /* isaretli  */
void oled_print_dec(int16_t v_x10); /* 1 ondalik: 245 -> "24.5" */

#endif /* OLED_H */
