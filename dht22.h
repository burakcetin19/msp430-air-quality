#ifndef DHT22_H
#define DHT22_H

#include <msp430.h>
#include <stdint.h>

/* DHT22 data pini: P2.0 */
#define DHT_PORT_DIR    P2DIR
#define DHT_PORT_OUT    P2OUT
#define DHT_PORT_IN     P2IN
#define DHT_PORT_REN    P2REN
#define DHT_PIN         BIT0

typedef struct {
    int16_t  temperature_dc;  /* 1/10 derece (245 = 24.5 C) */
    uint16_t humidity_dp;     /* 1/10 yuzde  (512 = 51.2 %) */
    uint8_t  valid;           /* 1 = okuma basarili */
} dht22_data_t;

void    dht22_init(void);
uint8_t dht22_read(dht22_data_t *data);

#endif /* DHT22_H */
