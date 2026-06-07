#include "mq_sensors.h"

/* ADC10:
 *   - Vcc / GND referans (SREF_0)
 *   - 64 cycle ornekleme (ADC10SHT_3) - MQ cikislari yuksek empedansli
 *   - SMCLK / 4 (ADC10SSEL_3, ADC10DIV_3)
 *
 * NOT: MQ sensorlerin gercek ppm degerine cevirimi log-egri ile yapilir.
 * Burada projeyi calistirip esik karari verebilmek icin LINEER bir
 * yaklasim kullaniliyor (ham ADC -> kabul edilebilir ppm araligi).
 * Hassas olcum istersen RL, Ro kalibrasyonu ile log10 formulu eklemek
 * gerekir. (Bkz. README "Kalibrasyon" basligi.)
 */

void mq_init(void)
{
    /* P1.3 ve P1.4 analog girise alinir (digital I/O devre disi). */
    ADC10AE0 |= MQ7_PIN | MQ135_PIN;
}

uint16_t mq_read_raw(uint16_t inch)
{
    ADC10CTL0 &= ~ENC;
    ADC10CTL1  = inch | ADC10SSEL_3 | ADC10DIV_3;       /* SMCLK/4 */
    ADC10CTL0  = SREF_0 | ADC10SHT_3 | ADC10ON;         /* Vcc ref, 64 cyc */
    __delay_cycles(200);                                /* settling */

    ADC10CTL0 |= ENC | ADC10SC;
    while (ADC10CTL1 & ADC10BUSY);

    return ADC10MEM;
}

uint16_t mq7_read_co_ppm(void)
{
    /* Lineer yaklasim: 0..1023 -> 0..1000 ppm CO */
    uint16_t raw = mq_read_raw(MQ7_INCH);
    return (uint16_t)((uint32_t)raw * 1000U / 1023U);
}

uint16_t mq135_read_gas_ppm(void)
{
    /* Lineer yaklasim: 0..1023 -> 0..2000 ppm CO2/karisik gaz */
    uint16_t raw = mq_read_raw(MQ135_INCH);
    return (uint16_t)((uint32_t)raw * 2000U / 1023U);
}
