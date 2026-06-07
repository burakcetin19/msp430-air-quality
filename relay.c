#include "relay.h"

static uint8_t s_state = 0;

void relay_init(void)
{
    RELAY_PORT_DIR |= RELAY_PIN;
    relay_off();
}

void relay_on(void)
{
#if RELAY_ACTIVE_LOW
    RELAY_PORT_OUT &= ~RELAY_PIN;
#else
    RELAY_PORT_OUT |= RELAY_PIN;
#endif
    s_state = 1;
}

void relay_off(void)
{
#if RELAY_ACTIVE_LOW
    RELAY_PORT_OUT |= RELAY_PIN;
#else
    RELAY_PORT_OUT &= ~RELAY_PIN;
#endif
    s_state = 0;
}

uint8_t relay_state(void)
{
    return s_state;
}
