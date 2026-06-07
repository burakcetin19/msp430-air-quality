#include "dht22.h"

/* DHT22 tek-tel protokolu, MCLK = 1 MHz varsayilir.
 * __delay_cycles(N) = N us
 *
 * Sira:
 *  1) MCU hatti 1.1 ms LOW cekiyor (start)
 *  2) MCU birakir, 20-40 us beklenir, hat HIGH
 *  3) DHT cevap: 80 us LOW + 80 us HIGH
 *  4) 40 bit: her bit 50 us LOW + (~28 us HIGH = 0, ~70 us HIGH = 1)
 *  5) Hat tekrar HIGH'a cekilir (idle)
 *
 * Veri biciminde:
 *   bytes[0..1] = nem * 10 (big-endian)
 *   bytes[2..3] = sicaklik * 10 (msbit = isaret)
 *   bytes[4]    = checksum = (b0+b1+b2+b3) & 0xFF
 */

#define DHT_HIGH()    (DHT_PORT_OUT |=  DHT_PIN)
#define DHT_LOW()     (DHT_PORT_OUT &= ~DHT_PIN)
#define DHT_OUTPUT()  (DHT_PORT_DIR |=  DHT_PIN)
#define DHT_INPUT()   (DHT_PORT_DIR &= ~DHT_PIN)
#define DHT_READ()    (DHT_PORT_IN  &   DHT_PIN)

void dht22_init(void)
{
    /* Idle = output high. Harici pull-up (4.7k-10k) onerilir. */
    DHT_OUTPUT();
    DHT_HIGH();
}

uint8_t dht22_read(dht22_data_t *data)
{
    uint8_t  bytes[5] = {0, 0, 0, 0, 0};
    uint16_t timeout;
    int16_t  temp;
    int8_t   i, j;

    data->valid = 0;

    /* 1) Start sinyali: ~1.1 ms LOW */
    DHT_OUTPUT();
    DHT_LOW();
    __delay_cycles(1100);

    /* 2) Hatti birak, ~30 us bekle (DHT cevap basliyor) */
    DHT_HIGH();
    DHT_INPUT();
    __delay_cycles(30);

    /* 3) Cevap: high (kisa), low ~80 us, high ~80 us */
    timeout = 150;
    while (DHT_READ() && --timeout) __delay_cycles(1);
    if (!timeout) goto fail;

    timeout = 150;
    while (!DHT_READ() && --timeout) __delay_cycles(1);
    if (!timeout) goto fail;

    timeout = 150;
    while (DHT_READ() && --timeout) __delay_cycles(1);
    if (!timeout) goto fail;

    /* 4) 40 bit oku */
    for (i = 0; i < 5; i++) {
        for (j = 7; j >= 0; j--) {
            /* 50 us LOW bekle - rising edge */
            timeout = 150;
            while (!DHT_READ() && --timeout) __delay_cycles(1);
            if (!timeout) goto fail;

            /* ~40 us sonra ornekle: high ise '1', dustuyse '0' */
            __delay_cycles(40);
            if (DHT_READ()) {
                bytes[i] |= (1 << j);
                /* Bit '1' ise hala HIGH'ta. Falling edge'i bekle. */
                timeout = 150;
                while (DHT_READ() && --timeout) __delay_cycles(1);
                if (!timeout) goto fail;
            }
        }
    }

    /* 5) Idle */
    DHT_OUTPUT();
    DHT_HIGH();

    /* Checksum */
    if ((uint8_t)(bytes[0] + bytes[1] + bytes[2] + bytes[3]) != bytes[4])
        return 0;

    data->humidity_dp = ((uint16_t)bytes[0] << 8) | bytes[1];

    temp = (int16_t)(((uint16_t)(bytes[2] & 0x7F) << 8) | bytes[3]);
    if (bytes[2] & 0x80) temp = -temp;
    data->temperature_dc = temp;

    data->valid = 1;
    return 1;

fail:
    DHT_OUTPUT();
    DHT_HIGH();
    return 0;
}
