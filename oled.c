#include <msp430.h>
#include "oled.h"
#include "i2c_b0.h"

#define OLED_I2C_ADDR     0x3C   /* bazi modullerinde 0x3D */

#define OLED_WIDTH        128
#define OLED_HEIGHT       32
#define OLED_PAGES        4

/* SSD1306 control byte: 0x00 = command stream, 0x40 = data stream */
#define OLED_CB_CMD       0x00
#define OLED_CB_DATA      0x40

static uint8_t cur_col  = 0;
static uint8_t cur_page = 0;

/* 5x7 font - sadece kullandigimiz karakter altkumesi.
 * Eksik karakter istenirse '?' yerine bosluk basilir.
 * Format: sutun bazli, bit 0 = en ust piksel.                 */

typedef struct {
    char    c;
    uint8_t cols[5];
} glyph_t;

static const glyph_t font_table[] = {
    {' ', {0x00, 0x00, 0x00, 0x00, 0x00}},
    {'.', {0x00, 0x60, 0x60, 0x00, 0x00}},
    {':', {0x00, 0x36, 0x36, 0x00, 0x00}},
    {'-', {0x08, 0x08, 0x08, 0x08, 0x08}},
    {'/', {0x20, 0x10, 0x08, 0x04, 0x02}},
    {'0', {0x3E, 0x51, 0x49, 0x45, 0x3E}},
    {'1', {0x00, 0x42, 0x7F, 0x40, 0x00}},
    {'2', {0x42, 0x61, 0x51, 0x49, 0x46}},
    {'3', {0x21, 0x41, 0x45, 0x4B, 0x31}},
    {'4', {0x18, 0x14, 0x12, 0x7F, 0x10}},
    {'5', {0x27, 0x45, 0x45, 0x45, 0x39}},
    {'6', {0x3C, 0x4A, 0x49, 0x49, 0x30}},
    {'7', {0x01, 0x71, 0x09, 0x05, 0x03}},
    {'8', {0x36, 0x49, 0x49, 0x49, 0x36}},
    {'9', {0x06, 0x49, 0x49, 0x29, 0x1E}},
    {'A', {0x7E, 0x11, 0x11, 0x11, 0x7E}},
    {'C', {0x3E, 0x41, 0x41, 0x41, 0x22}},
    {'E', {0x7F, 0x49, 0x49, 0x49, 0x41}},
    {'F', {0x7F, 0x09, 0x09, 0x01, 0x01}},
    {'G', {0x3E, 0x41, 0x41, 0x51, 0x32}},
    {'H', {0x7F, 0x08, 0x08, 0x08, 0x7F}},
    {'I', {0x00, 0x41, 0x7F, 0x41, 0x00}},
    {'O', {0x3E, 0x41, 0x41, 0x41, 0x3E}},
    {'Q', {0x3E, 0x41, 0x51, 0x21, 0x5E}},
    {'R', {0x7F, 0x09, 0x19, 0x29, 0x46}},
    {'S', {0x46, 0x49, 0x49, 0x49, 0x31}},
    {'T', {0x01, 0x01, 0x7F, 0x01, 0x01}},
};

#define FONT_TABLE_SIZE  (sizeof(font_table) / sizeof(font_table[0]))

static const uint8_t* glyph_for(char c)
{
    uint8_t i;
    static const uint8_t blank[5] = {0,0,0,0,0};
    for (i = 0; i < FONT_TABLE_SIZE; i++) {
        if (font_table[i].c == c) return font_table[i].cols;
    }
    return blank;
}

/* --- SSD1306 alt-seviye --- */

static void oled_cmd(uint8_t c)
{
    uint8_t buf[2] = { OLED_CB_CMD, c };
    i2c_b0_set_slave(OLED_I2C_ADDR);
    i2c_b0_write(buf, 2);
}

static void oled_cmd2(uint8_t c1, uint8_t c2)
{
    uint8_t buf[3] = { OLED_CB_CMD, c1, c2 };
    i2c_b0_set_slave(OLED_I2C_ADDR);
    i2c_b0_write(buf, 3);
}

/* Verileri tek I2C transferinde yolla: control + N data */
static void oled_data(const uint8_t *data, uint16_t len)
{
    /* control byte + data: tek transfer icin baslangic bayti */
    uint8_t hdr[1] = { OLED_CB_DATA };
    i2c_b0_set_slave(OLED_I2C_ADDR);

    /* Su anki i2c_b0 API'si tek pasla yaziyor. Kucuk veri icin
     * (per-char 6 byte) tek transfer icine sigdiriyoruz.        */
    uint8_t packet[16];
    uint16_t i;
    if (len + 1 > sizeof(packet)) return;     /* guvenlik */
    packet[0] = OLED_CB_DATA;
    for (i = 0; i < len; i++) packet[i + 1] = data[i];
    i2c_b0_write(packet, len + 1);
    (void)hdr;
}

static void oled_set_column(uint8_t col)
{
    oled_cmd(0x00 | (col & 0x0F));         /* lower nibble  */
    oled_cmd(0x10 | ((col >> 4) & 0x0F));  /* higher nibble */
}

static void oled_set_page(uint8_t page)
{
    oled_cmd(0xB0 | (page & 0x07));
}

/* --- Public API --- */

void oled_init(void)
{
    /* SSD1306 dahili olarak ~100 ms reset bekleyebilir */
    uint16_t i;
    for (i = 0; i < 100; i++) __delay_cycles(1000);

    oled_cmd(0xAE);              /* display off */
    oled_cmd2(0xD5, 0x80);       /* clock divide */
    oled_cmd2(0xA8, 0x1F);       /* multiplex 32-1 = 0x1F */
    oled_cmd2(0xD3, 0x00);       /* display offset */
    oled_cmd(0x40);              /* start line = 0 */
    oled_cmd2(0x8D, 0x14);       /* charge pump on */
    oled_cmd2(0x20, 0x00);       /* horizontal addressing */
    oled_cmd(0xA1);              /* segment remap (flip horiz) */
    oled_cmd(0xC8);              /* COM scan dec (flip vert)   */
    oled_cmd2(0xDA, 0x02);       /* COM pins for 128x32        */
    oled_cmd2(0x81, 0x8F);       /* contrast */
    oled_cmd2(0xD9, 0xF1);       /* precharge */
    oled_cmd2(0xDB, 0x40);       /* vcom detect */
    oled_cmd(0xA4);              /* display ALL on RAM */
    oled_cmd(0xA6);              /* normal (not inverse) */
    oled_cmd(0xAF);              /* display on */

    oled_clear();
}

void oled_clear(void)
{
    uint8_t page, col;
    uint8_t zeros[16];
    for (page = 0; page < OLED_PAGES; page++) zeros[page] = 0;
    /* zeros bufferinin tamamini sifirla */
    for (col = 0; col < sizeof(zeros); col++) zeros[col] = 0;

    for (page = 0; page < OLED_PAGES; page++) {
        oled_set_page(page);
        oled_set_column(0);
        /* 128 sutunu 16'lik bloklar halinde gonder */
        for (col = 0; col < OLED_WIDTH; col += sizeof(zeros) - 1) {
            uint8_t chunk = (uint8_t)((OLED_WIDTH - col) < (sizeof(zeros) - 1)
                                       ? (OLED_WIDTH - col)
                                       : (sizeof(zeros) - 1));
            oled_data(zeros, chunk);
        }
    }

    cur_col = 0;
    cur_page = 0;
}

void oled_set_cursor(uint8_t col, uint8_t page)
{
    if (col  >= OLED_WIDTH) col  = OLED_WIDTH - 1;
    if (page >= OLED_PAGES) page = OLED_PAGES - 1;
    cur_col  = col;
    cur_page = page;
    oled_set_page(page);
    oled_set_column(col);
}

void oled_print_char(char c)
{
    uint8_t buf[6];
    const uint8_t *g;
    uint8_t i;

    if (cur_col + 6 > OLED_WIDTH) return;     /* satir sonu, sigmiyor */

    g = glyph_for(c);
    for (i = 0; i < 5; i++) buf[i] = g[i];
    buf[5] = 0;                                 /* 1 piksel bosluk */
    oled_data(buf, 6);
    cur_col += 6;
}

void oled_print_str(const char *s)
{
    while (*s) oled_print_char(*s++);
}

void oled_print_num(uint16_t n)
{
    char buf[6];
    int8_t i = 0;
    if (n == 0) { oled_print_char('0'); return; }
    while (n > 0) {
        buf[i++] = (char)((n % 10) + '0');
        n /= 10;
    }
    while (--i >= 0) oled_print_char(buf[i]);
}

void oled_print_int(int16_t n)
{
    if (n < 0) { oled_print_char('-'); n = -n; }
    oled_print_num((uint16_t)n);
}

void oled_print_dec(int16_t v_x10)
{
    int16_t whole = v_x10 / 10;
    int16_t frac  = v_x10 % 10;
    if (frac < 0) frac = -frac;
    oled_print_int(whole);
    oled_print_char('.');
    oled_print_char((char)('0' + frac));
}
