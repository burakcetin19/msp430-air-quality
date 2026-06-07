#include "bluetooth.h"
#include "uart.h"

/* HC-05 = seri kanal. Tek isi USCI_A0'i 9600 8N1 @ 1 MHz olarak ac.
 * Init sirasi uart.c icinde dondurulmus durumda; buradan bozulmasi
 * mumkun degil.                                                       */
void bt_init(void)
{
    uart_init_9600_1mhz();
}

void bt_send_hello(void)
{
    uart_puts("\r\n[MSP430] Hava Kalitesi Olcum baslatildi\r\n");
}

/* Yardimcilar: 1 ondalik hassasiyetli sayilari yaz. */

static void send_one_decimal_u(uint16_t value_x10)
{
    uart_putu(value_x10 / 10);
    uart_putc('.');
    uart_putc((char)('0' + (value_x10 % 10)));
}

static void send_one_decimal_i(int16_t value_x10)
{
    int16_t whole = value_x10 / 10;
    int16_t frac  = value_x10 % 10;
    if (frac < 0) frac = -frac;
    uart_puti(whole);
    uart_putc('.');
    uart_putc((char)('0' + frac));
}

void bt_send_packet(uint16_t co_ppm,
                    uint16_t gas_ppm,
                    int16_t  temp_dc,
                    uint16_t hum_dp,
                    uint8_t  sensors_valid,
                    uint8_t  fan_on)
{
    uart_puts("CO:");
    uart_putu(co_ppm);

    uart_puts(",GAS:");
    uart_putu(gas_ppm);

    uart_puts(",T:");
    if (sensors_valid) send_one_decimal_i(temp_dc);
    else               uart_puts("ERR");

    uart_puts(",H:");
    if (sensors_valid) send_one_decimal_u(hum_dp);
    else               uart_puts("ERR");

    uart_puts(",F:");
    uart_putc(fan_on ? '1' : '0');

    uart_puts("\r\n");
}
