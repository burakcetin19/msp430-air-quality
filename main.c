#include <msp430.h>
#include <stdint.h>

#include "config.h"
#include "bluetooth.h"
#include "dht22.h"
#include "mq_sensors.h"
#include "relay.h"
#include "aqi.h"
#include "flash.h"

/* ============================================================
 * TANI - WATCH ile okunabilir RAM snapshot'lar.
 *
 * MSP430G2553'te Info Segment A'daki CAL bayatlari okuyabilmek
 * icin debugger'in Memory/WATCH'ina guvenmek yetmiyor (CCS Theia
 * surumunde dereference bug'i var). Bu globalleri WATCH'a ekle:
 *
 *   snapshot_cal_bc1   -> normalde 0x86, 0x8E, 0x96 gibi degerler.
 *   snapshot_cal_dco   -> normalde 0xB5, 0xBD gibi degerler.
 *
 * Her ikisi de 0xFF ise -> Segment A silinmis, CAL kayip.
 * ============================================================ */
volatile uint8_t snapshot_cal_bc1 = 0;
volatile uint8_t snapshot_cal_dco = 0;

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
 *  RST butonu:
 *    Her basis BT VERI GONDERIMINI ac/kapat olarak toggle eder.
 *    Durum flash'a yazilir; guc kesilse bile korunur.
 *    TX kapali iken: sensorler okunmaya devam, fan kontrolu calisir,
 *                    sadece BT'ye satir gitmez.
 *
 *  Fan karari (her dongude):
 *    1) AQI histerezisi
 *         AQI > AQI_FAN_ON_THRESHOLD  -> on
 *         AQI < AQI_FAN_OFF_THRESHOLD -> off
 *         arada                       -> mevcut durumu koru
 *    2) Sicaklik override (TEMP_FAN_ON_DC) -> AQI'den bagimsiz on
 * ============================================================ */

static void delay_ms(uint16_t ms)
{
    /* MCLK = 1 MHz (uart_init_9600_1mhz icinde ayarlandi) */
    while (ms--) __delay_cycles(1000);
}

int main(void)
{
    dht22_data_t dht;
    uint16_t co_ppm, gas_ppm, aqi;
    uint8_t  fan_on    = 0;
    uint8_t  tx_enable = 0;

    WDTCTL = WDTPW | WDTHOLD;

    /* CAL bayatlarinin RAM kopyasi - tani amacli */
    snapshot_cal_bc1 = CALBC1_1MHZ;
    snapshot_cal_dco = CALDCO_1MHZ;

    /* bt_init() icinde DCO 1 MHz'e cekiliyor; en once cagirilmali.
     * TX kapali olsa bile UART'i kurmak zararsiz; sadece veri gitmez. */
    bt_init();
    dht22_init();
    mq_init();
    relay_init();

    /* Bu boot bir TOGGLE'dir (config.h ENABLE_TX_TOGGLE = 1 iken).
     * Cip ilk programlandiktan sonraki ilk acilis: TX = ON.
     * Sonraki her RST basisi: 0->1 / 1->0.
     *
     * Test/tanı modu: ENABLE_TX_TOGGLE = 0 yapilirsa flash operasyonu
     * hic yapilmaz, TX hep acik kalir. BT garbage incelerken flash
     * op'unu sorundan izole etmek icin kullan.                       */
#if ENABLE_TX_TOGGLE
    tx_enable = flash_toggle_tx_flag();
#else
    tx_enable = 1;
#endif

    if (tx_enable) {
        bt_send_hello();
        bt_send_tx_status(1);
    }

    /* MQ sensorler icin kisa isinma */
    delay_ms(WARMUP_PERIOD_MS);

    while (1) {
        /* --- Olcumler --- */
        co_ppm  = mq7_read_co_ppm();
        gas_ppm = mq135_read_gas_ppm();
        dht22_read(&dht);

        aqi = aqi_calc(co_ppm, gas_ppm);

        /* --- Fan karari: AQI histerezisi --- */
        if      (aqi > AQI_FAN_ON_THRESHOLD)  fan_on = 1;
        else if (aqi < AQI_FAN_OFF_THRESHOLD) fan_on = 0;
        /* arada: fan_on degismez */

        /* Sicaklik override (sadece "on" yonunde, off karari AQI'nin) */
        if (dht.valid && dht.temperature_dc > TEMP_FAN_ON_DC)
            fan_on = 1;

        if (fan_on) relay_on();
        else        relay_off();

        /* --- BT (sadece TX acikken) --- */
        if (tx_enable) {
            bt_send_packet(aqi,
                           co_ppm,
                           gas_ppm,
                           dht.temperature_dc,
                           dht.humidity_dp,
                           dht.valid,
                           relay_state());
        }

        delay_ms(READING_PERIOD_MS);
    }
}
