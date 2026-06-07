#include "bluetooth.h"

/* HC-05 UART init.
 * Test kodundaki sira korunmustur:
 *   1) Pin fonksiyonlari (P1SEL / P1SEL2)
 *   2) DCO -> 1 MHz kalibre
 *   3) UART konfigurasyonu (UCSWRST altinda)
 */
void bt_init(void)
{
    /* 1) Pin fonksiyonlari */
    P1SEL  |= BT_RX | BT_TX;
    P1SEL2 |= BT_RX | BT_TX;

    /* 2) Clock - 1 MHz kalibre DCO */
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL  = CALDCO_1MHZ;

    /* 3) UART - 9600 baud @ 1 MHz */
    UCA0CTL1 |= UCSWRST;          /* reset durumunda yapilandir */
    UCA0CTL1 |= UCSSEL_2;         /* SMCLK kaynagi */
    UCA0BR0   = 104;              /* 1.000.000 / 9600 = 104.16 */
    UCA0BR1   = 0;
    UCA0MCTL  = UCBRS_1;          /* kesir ~0.16 -> UCBRS=1 */
    UCA0CTL1 &= ~UCSWRST;         /* UART'i aktif et */
}

void bt_send_char(char c)
{
    while (!(IFG2 & UCA0TXIFG));
    UCA0TXBUF = c;
}

void bt_send_string(const char *str)
{
    while (*str) bt_send_char(*str++);
}

void bt_send_number(uint16_t num)
{
    char buf[6];
    int8_t i = 0;

    if (num == 0) {
        bt_send_char('0');
        return;
    }
    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    while (--i >= 0) bt_send_char(buf[i]);
}

void bt_send_signed(int16_t num)
{
    if (num < 0) {
        bt_send_char('-');
        num = -num;
    }
    bt_send_number((uint16_t)num);
}
