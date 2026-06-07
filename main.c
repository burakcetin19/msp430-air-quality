#include <msp430.h>
#include <stdint.h>

#include "config.h"
#include "bluetooth.h"
#include "dht22.h"
#include "mq_sensors.h"
#include "relay.h"
#include "aqi.h"
#include "i2c_b0.h"
#include "oled.h"

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
    uint16_t co_ppm, gas_ppm, aqi;
    uint8_t  fan = 0;     /* histerezis icin baslangic durumunu sakla */

    WDTCTL = WDTPW | WDTHOLD;

    /* bt_init() ICINDEKI uart_init_9600_1mhz() DCO'yu 1 MHz'e ceker.
     * Bu yuzden EN BASTA cagrilmalidir; diger init'ler bu clock'i
     * varsayar (delay_ms, DHT22 timing, ADC sampling).               */
    bt_init();
    dht22_init();
    mq_init();
    relay_init();
    i2c_b0_init();   /* OLED haberlesmesi icin USCI_B0 (P1.6/P1.7) */
    oled_init();     /* SSD1306 acilis sekansi                      */

    bt_send_hello();

    /* MQ sensorleri icin kisa isinma  */
    delay_ms(WARMUP_PERIOD_MS);

    while (1) {
        /* --- Olcumler --- */
        co_ppm  = mq7_read_co_ppm();
        gas_ppm = mq135_read_gas_ppm();
        dht22_read(&dht);

        aqi = aqi_calc(co_ppm, gas_ppm);

        /* --- Fan karari: AQI histerezisi + sicaklik override --- */
        if      (aqi > AQI_FAN_ON_THRESHOLD)  fan = 1;
        else if (aqi < AQI_FAN_OFF_THRESHOLD) fan = 0;
        /* esikler arasinda fan degeri korunur */

        if (dht.valid && dht.temperature_dc > TEMP_THRESHOLD_DC)
            fan = 1;

        if (fan) relay_on();
        else     relay_off();

        /* --- BT paketi --- */
        bt_send_packet(aqi,
                       co_ppm,
                       gas_ppm,
                       dht.temperature_dc,
                       dht.humidity_dp,
                       dht.valid,
                       relay_state());

        /* --- OLED ekrani --- */
        oled_clear();

        oled_set_cursor(0, 0);
        oled_print_str("AQI:");
        oled_print_num(aqi);
        oled_print_str("  F:");
        oled_print_num(relay_state());

        oled_set_cursor(0, 1);
        oled_print_str("CO:");
        oled_print_num(co_ppm);
        oled_print_str(" GAS:");
        oled_print_num(gas_ppm);

        oled_set_cursor(0, 2);
        if (dht.valid) {
            oled_print_str("T:");
            oled_print_dec(dht.temperature_dc);
            oled_print_str(" H:");
            oled_print_dec(dht.humidity_dp);
        } else {
            oled_print_str("T:ERR H:ERR");
        }

        delay_ms(READING_PERIOD_MS);
    }
}
