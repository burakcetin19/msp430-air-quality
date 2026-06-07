#include <msp430.h>
#include "flash.h"

/* Info Segment D = 0x1000 - 0x103F (64 byte).
 * Segment A (0x10C0) kalibrasyon icin, asla yazma!
 */
#define FLASH_USER_ADDR   ((uint8_t *)0x1000)

#define FLAG_TX_ON        0xA5
#define FLAG_TX_OFF       0x5A
#define FLAG_ERASED       0xFF

uint8_t flash_read_tx_flag(void)
{
    uint8_t v = *FLASH_USER_ADDR;
    /* Bilinmeyen / yeni programlanmis (0xFF) -> ACIK */
    if (v == FLAG_TX_OFF) return 0;
    return 1;
}

void flash_write_tx_flag(uint8_t on)
{
    uint8_t val = on ? FLAG_TX_ON : FLAG_TX_OFF;

    /* Flash timing generator: SMCLK / (FN1+1) = 1 MHz / 3 = 333 kHz
     * (gecerli aralik 257-476 kHz)                                  */
    FCTL2 = FWKEY | FSSEL_2 | FN1;

    __disable_interrupt();
    while (FCTL3 & BUSY);

    FCTL3 = FWKEY;                       /* Unlock                 */
    FCTL1 = FWKEY | ERASE;               /* Erase modu             */
    *FLASH_USER_ADDR = 0;                /* dummy yazi - erase tetik */
    while (FCTL3 & BUSY);

    FCTL1 = FWKEY | WRT;                 /* Write modu             */
    *FLASH_USER_ADDR = val;
    while (FCTL3 & BUSY);

    FCTL1 = FWKEY;                       /* Mod cikis              */
    FCTL3 = FWKEY | LOCK;                /* Lock                   */

    __enable_interrupt();
}

uint8_t flash_toggle_tx_flag(void)
{
    uint8_t v = *FLASH_USER_ADDR;
    uint8_t new_val;

    if (v == FLAG_ERASED) {
        /* Cipi ilk acislari - varsayilan ACIK, toggle yok */
        new_val = 1;
    } else if (v == FLAG_TX_ON) {
        new_val = 0;
    } else {
        new_val = 1;
    }

    flash_write_tx_flag(new_val);
    return new_val;
}
