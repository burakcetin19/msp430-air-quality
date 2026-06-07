#ifndef CONFIG_H
#define CONFIG_H

/* ============================================================
 * Hava Kalitesi Olcum Projesi - MSP430G2553
 *
 * Esik degerleri ve global ayarlar.
 * ============================================================ */

/* --- Fan kontrol: AQI histerezisi ---
 *
 *  AQI > AQI_FAN_ON_THRESHOLD  -> fan AC
 *  AQI < AQI_FAN_OFF_THRESHOLD -> fan KAPAT
 *  ARADA                        -> mevcut durumu KORU
 *
 *  Bu sayede AQI tam esikte titrerken role saniyede bir
 *  acilip kapanmaz.
 */
#define AQI_FAN_ON_THRESHOLD     150U   /* "Unhealthy for sensitive" usti */
#define AQI_FAN_OFF_THRESHOLD    100U   /* "Moderate" alti                */

/* --- Sicaklik override (gerekirse) ---
 *
 *  Sicaklik bu esigi gecerse, AQI ne olursa olsun fan acilir.
 *  Soguma ise AQI histerezisine birakilir.
 */
#define TEMP_FAN_ON_DC           300    /* 30.0 C */

/* --- Periyotlar (ms) --- */
#define READING_PERIOD_MS        2000U
#define WARMUP_PERIOD_MS         3000U

/* --- TX toggle test modu ---
 *
 *  1 (default) : RST basisi flash'taki bayragi cevirir, TX on/off.
 *  0           : Flash operasyonu hic yapilmaz, TX hep ACIK.
 *                BT garbage tanisinda flash op'u izole etmek icin.
 */
#define ENABLE_TX_TOGGLE         1

#endif /* CONFIG_H */
