#include <msp430.h>
#include <stdint.h>

#include "config.h"
#include "bluetooth.h"
#include "dht22.h"
#include "mq_sensors.h"
#include "relay.h"

/* ============================================================
 * MSP430G2553 - Hava Kalitesi Olcum ve Fan Kontrol Projesi
 *
 *  Sensorler:
 *    - MQ-7   (CO)     -> P1.4 / A4
 *    - MQ-135 (gaz)    -> P1.3 / A3
 *    - DHT22  (T/RH)   -> P2.0
 *
 *  Cikis / haberlesme:
 *    - Role (fan)      -> P2.1
 *    - HC-05 BT (UART) -> P1.1=RX, P1.2=TX, 9600 baud
 *
 *  Esikler config.h icinde.
 *  Esiklerden biri asilirsa role aktif olur.
 * ============================================================ */

static void delay_ms(uint16_t ms)
{
    /* MCLK = 1 MHz (bt_init -> uart_init_9600_1mhz icinde ayarlandi) */
    while (ms--) __delay_cycles(1000);
}

int main(void)
{
    dht22_data_t dht;
    uint16_t co_ppm, gas_ppm;
    uint8_t  fan;

    WDTCTL = WDTPW | WDTHOLD;

    /* bt_init() ICINDEKI uart_init_9600_1mhz() DCO'yu 1 MHz'e ceker.
     * Bu yuzden EN BASTA cagrilmalidir; diger init'ler bu clock'i
     * varsayar (delay_ms, DHT22 timing, ADC sampling).               */
    bt_init();
    dht22_init();
    mq_init();
    relay_init();

    bt_send_hello();

    /* MQ sensorleri icin kisa isinma  */
    delay_ms(WARMUP_PERIOD_MS);

    while (1) {
        /* --- Olcumler --- */
        co_ppm  = mq7_read_co_ppm();
        gas_ppm = mq135_read_gas_ppm();
        dht22_read(&dht);

        /* --- Esik kontrolu / fan karari --- */
        fan = 0;
        if (co_ppm  > CO_THRESHOLD_PPM)                          fan = 1;
        if (gas_ppm > GAS_THRESHOLD_PPM)                         fan = 1;
        if (dht.valid && dht.temperature_dc > TEMP_THRESHOLD_DC) fan = 1;

        if (fan) relay_on();
        else     relay_off();

        /* --- BT paketi --- */
        bt_send_packet(co_ppm,
                       gas_ppm,
                       dht.temperature_dc,
                       dht.humidity_dp,
                       dht.valid,
                       relay_state());

        delay_ms(READING_PERIOD_MS);
    }
}
