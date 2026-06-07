#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

/* MSP430G2553 Info Segment D (0x1000) icinde tek baytlik
 * "transmit enabled" bayragi.
 *
 * Boot anlaminda her kullanim:
 *   flash_toggle_tx_flag() -> her resetiste durumu cevirir, yeni
 *                             durumu doner (1 = TX acik, 0 = kapali).
 *
 * Ilk programlamadan sonraki ilk acilis varsayilan TX ACIK olarak
 * baslar (toggle yapmaz, sadece flash'a "ACIK" yazar).
 *
 * Wear notu: flash 10.000+ erase cycle dayanir, normal kullanimda
 * sorun olmaz.
 */

uint8_t flash_read_tx_flag(void);          /* 1 = on, 0 = off       */
void    flash_write_tx_flag(uint8_t on);   /* on=0/1                */
uint8_t flash_toggle_tx_flag(void);        /* toggle, yeni degeri dondurur */

#endif /* FLASH_H */
