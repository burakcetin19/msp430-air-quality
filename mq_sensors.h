#ifndef MQ_SENSORS_H
#define MQ_SENSORS_H

#include <msp430.h>
#include <stdint.h>

/* ADC kanallari:
 *   MQ-7   AOUT -> P1.4 -> A4
 *   MQ-135 AOUT -> P1.3 -> A3
 */
#define MQ7_INCH     INCH_4
#define MQ135_INCH   INCH_3

#define MQ7_PIN      BIT4
#define MQ135_PIN    BIT3

void     mq_init(void);
uint16_t mq_read_raw(uint16_t inch);
uint16_t mq7_read_co_ppm(void);
uint16_t mq135_read_gas_ppm(void);

#endif /* MQ_SENSORS_H */
