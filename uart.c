#include "uart.h"

/* =====================================================================
 * !!! DOKUNMA - TEST EDILMIS / CALISAN INIT SIRASI !!!
 *
 * MSP430G2553 USCI_A0 UART (P1.1=RX, P1.2=TX, 9600 8N1).
 *
 * Sira ASLA degistirilmemeli:
 *
 *   1) PIN FONKSIYONLARI   : P1SEL | P1SEL2 -> USCI'ya teslim et.
 *   2) CLOCK KALIBRASYONU  : BCSCTL1=CALBC1_1MHZ, DCOCTL=CALDCO_1MHZ.
 *                            Bu satirlar OLMASIN -> SMCLK kalibresiz
 *                            kalir -> baud yanlis -> "@^@^..." copluk.
 *   3) UART AYARI          : UCSWRST altinda OR'lama ile.
 *
 *   KRITIK:
 *     UCA0CTL1  = UCSWRST;   // YANLIS - atama, diger bitleri sifirlar
 *     UCA0CTL1 |= UCSWRST;   // DOGRU  - OR, sadece reset bitini set eder
 *
 * Bu kodun bu hali ile cikti temiz aliniyor. Yeni bir feature eklemek
 * istiyorsan altta yeni fonksiyon ac, uart_init_9600_1mhz() govdesine
 * dokunma.
 * ===================================================================== */

#define UART_RX_PIN   BIT1   /* P1.1 = UCA0RXD */
#define UART_TX_PIN   BIT2   /* P1.2 = UCA0TXD */

void uart_init_9600_1mhz(void)
{
    /* 1) Pin fonksiyonlari */
    P1SEL  |= UART_RX_PIN | UART_TX_PIN;
    P1SEL2 |= UART_RX_PIN | UART_TX_PIN;

    /* 2) Clock - 1 MHz kalibre DCO (SMCLK boylece 1 MHz olur) */
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL  = CALDCO_1MHZ;

    /* 3) UART konfigurasyonu - UCSWRST altinda */
    UCA0CTL1 |= UCSWRST;           /* SOFT reset (OR ! atama degil) */
    UCA0CTL1 |= UCSSEL_2;          /* SMCLK kaynagi */
    UCA0BR0   = 104;               /* 1.000.000 / 9600 = 104.16    */
    UCA0BR1   = 0;
    UCA0MCTL  = UCBRS_1;           /* kesir ~0.16 -> UCBRS = 1     */
    UCA0CTL1 &= ~UCSWRST;          /* soft reset cik -> UART aktif */
}

void uart_putc(char c)
{
    while (!(IFG2 & UCA0TXIFG));
    UCA0TXBUF = c;
}

void uart_puts(const char *s)
{
    while (*s) uart_putc(*s++);
}

void uart_putu(uint16_t n)
{
    char    buf[6];
    int8_t  i = 0;

    if (n == 0) {
        uart_putc('0');
        return;
    }
    while (n > 0) {
        buf[i++] = (char)((n % 10) + '0');
        n /= 10;
    }
    while (--i >= 0) uart_putc(buf[i]);
}

void uart_puti(int16_t n)
{
    if (n < 0) {
        uart_putc('-');
        n = -n;
    }
    uart_putu((uint16_t)n);
}
