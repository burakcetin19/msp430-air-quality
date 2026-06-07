#include <msp430.h>
#include "i2c_b0.h"

/* USCI_B0 I2C master, 100 kHz @ SMCLK=1MHz.
 *
 * Pin yerleseimi:
 *   P1.6 = UCB0SCL
 *   P1.7 = UCB0SDA
 *
 * UART (USCI_A0) ile clock ve pinler bagimsiz - BT'yi etkilemez.
 */

#define I2C_SCL_PIN   BIT6
#define I2C_SDA_PIN   BIT7

void i2c_b0_init(void)
{
    /* Pinleri USCI_B0'a teslim et */
    P1SEL  |= I2C_SCL_PIN | I2C_SDA_PIN;
    P1SEL2 |= I2C_SCL_PIN | I2C_SDA_PIN;

    /* USCI_B0 konfigurasyonu UCSWRST altinda */
    UCB0CTL1 |= UCSWRST;
    UCB0CTL0  = UCMST | UCMODE_3 | UCSYNC;  /* master, I2C, sync */
    UCB0CTL1  = UCSSEL_2 | UCSWRST;         /* SMCLK kaynagi */
    UCB0BR0   = 10;                          /* 1 MHz / 10 = 100 kHz */
    UCB0BR1   = 0;
    UCB0CTL1 &= ~UCSWRST;                    /* aktif */
}

void i2c_b0_set_slave(uint8_t addr_7bit)
{
    UCB0I2CSA = addr_7bit;
}

uint8_t i2c_b0_write(const uint8_t *data, uint16_t len)
{
    uint16_t i;

    /* Onceki STOP'un bitmesini bekle */
    while (UCB0CTL1 & UCTXSTP);

    /* TX modu + START */
    UCB0CTL1 |= UCTR | UCTXSTT;

    for (i = 0; i < len; i++) {
        /* TX buffer bos olana kadar bekle veya NACK kontrol et */
        while (!(IFG2 & UCB0TXIFG)) {
            if (UCB0STAT & UCNACKIFG) {
                UCB0STAT &= ~UCNACKIFG;
                UCB0CTL1 |= UCTXSTP;
                while (UCB0CTL1 & UCTXSTP);
                return 0;
            }
        }
        UCB0TXBUF = data[i];
    }

    /* Son baytin gonderilmesini bekle */
    while (!(IFG2 & UCB0TXIFG)) {
        if (UCB0STAT & UCNACKIFG) {
            UCB0STAT &= ~UCNACKIFG;
            UCB0CTL1 |= UCTXSTP;
            while (UCB0CTL1 & UCTXSTP);
            return 0;
        }
    }

    /* STOP */
    UCB0CTL1 |= UCTXSTP;
    while (UCB0CTL1 & UCTXSTP);

    IFG2 &= ~UCB0TXIFG;
    return 1;
}
