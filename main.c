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
 *  Esikler config.h icinde tanimli.
 *  Esiklerden birisi asilirsa role aktif olur.
 * ============================================================ */

static void delay_ms(uint16_t ms)
{
    /* MCLK = 1 MHz */
    while (ms--) __delay_cycles(1000);
}

int main(void)
{
    dht22_data_t dht;
    uint16_t co_ppm, gas_ppm;
    uint8_t  fan;

    WDTCTL = WDTPW | WDTHOLD;

    /* bt_init() icinde DCO 1 MHz'e cekiliyor; en once cagrilmasi
     * sonraki delay_ms / DHT22 zamanlamasi acisindan onemli.       */
    bt_init();
    dht22_init();
    mq_init();
    relay_init();

    bt_send_string("\r\n[MSP430] Hava Kalitesi Olcum baslatildi\r\n");

    /* MQ sensorler isinma sureti istiyor */
    delay_ms(WARMUP_PERIOD_MS);

    while (1) {
        /* --- Olcumler --- */
        co_ppm  = mq7_read_co_ppm();
        gas_ppm = mq135_read_gas_ppm();

        /* DHT22 saniyede bir okunabilir; ana donguden ~2s gectigi
         * icin sorun yok. Hata durumunda son fan kararini koruyoruz.*/
        dht22_read(&dht);

        /* --- Esik kontrolu / fan karari --- */
        fan = 0;
        if (co_ppm  > CO_THRESHOLD_PPM)        fan = 1;
        if (gas_ppm > GAS_THRESHOLD_PPM)       fan = 1;
        if (dht.valid && dht.temperature_dc > TEMP_THRESHOLD_DC) fan = 1;

        if (fan) relay_on();
        else     relay_off();

        /* --- BT veri paketi ---
         * Bicim:  CO:<ppm>,GAS:<ppm>,T:<xx.x>,H:<xx.x>,F:<0|1>\r\n
         */
        bt_send_string("CO:");
        bt_send_number(co_ppm);

        bt_send_string(",GAS:");
        bt_send_number(gas_ppm);

        bt_send_string(",T:");
        if (dht.valid) {
            int16_t t = dht.temperature_dc;
            int16_t whole = t / 10;
            int16_t frac  = t % 10;
            if (frac < 0) frac = -frac;
            bt_send_signed(whole);
            bt_send_char('.');
            bt_send_char((char)('0' + frac));
        } else {
            bt_send_string("ERR");
        }

        bt_send_string(",H:");
        if (dht.valid) {
            bt_send_number(dht.humidity_dp / 10);
            bt_send_char('.');
            bt_send_char((char)('0' + (dht.humidity_dp % 10)));
        } else {
            bt_send_string("ERR");
        }

        bt_send_string(",F:");
        bt_send_char(relay_state() ? '1' : '0');
        bt_send_string("\r\n");

        delay_ms(READING_PERIOD_MS);
    }
}
