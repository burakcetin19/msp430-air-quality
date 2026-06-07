#ifndef UART_H
#define UART_H

#include <msp430.h>
#include <stdint.h>

/* MSP430G2553 USCI_A0 UART surucusu.
 * Sabit konfigurasyon: 9600 baud, 8N1, SMCLK = 1 MHz DCO.
 * Pinler: P1.1 = UCA0RXD, P1.2 = UCA0TXD
 */

void uart_init_9600_1mhz(void);

/* 1 = Segment A kalibrasyonu mevcut (DCO hassas)
 * 0 = silinmis, fallback DCO kullanildi (baud sinirda)            */
uint8_t uart_calibration_ok(void);

void uart_putc(char c);
void uart_puts(const char *s);
void uart_putu(uint16_t n);   /* isaretsiz decimal */
void uart_puti(int16_t  n);   /* isaretli   decimal */

#endif /* UART_H */
