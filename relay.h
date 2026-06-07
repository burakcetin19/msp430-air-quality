#ifndef RELAY_H
#define RELAY_H

#include <msp430.h>
#include <stdint.h>

/* Role kontrol pini: P2.1 (active-high varsayilir).
 * Eger role karti active-low ise RELAY_ACTIVE_LOW'i 1 yapin.
 */
#define RELAY_PORT_DIR   P2DIR
#define RELAY_PORT_OUT   P2OUT
#define RELAY_PIN        BIT1

#define RELAY_ACTIVE_LOW 0

void    relay_init(void);
void    relay_on(void);
void    relay_off(void);
uint8_t relay_state(void);

#endif /* RELAY_H */
