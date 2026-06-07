#ifndef I2C_B0_H
#define I2C_B0_H

#include <stdint.h>

/* MSP430G2553 USCI_B0 I2C master.
 * Pinler: P1.6 = SCL, P1.7 = SDA (donanim pinleri).
 * 100 kHz @ SMCLK = 1 MHz.
 *
 * UART'tan (USCI_A0) tamamen bagimsizdir, BT'ye dokunmaz.
 */

void    i2c_b0_init(void);
void    i2c_b0_set_slave(uint8_t addr_7bit);

/* len bayti slave'e gonder. Basarili ise 1, NACK aldiysa 0 doner. */
uint8_t i2c_b0_write(const uint8_t *data, uint16_t len);

#endif /* I2C_B0_H */
