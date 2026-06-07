#ifndef CONFIG_H
#define CONFIG_H

/* ============================================================
 * Hava Kalitesi Olcum Projesi - MSP430G2553
 *
 * Esik degerleri ve global ayarlar.
 * ============================================================ */

/* CO esigi (ppm) - MQ-7  */
#define CO_THRESHOLD_PPM        50

/* Gaz / CO2 esigi (ppm) - MQ-135 */
#define GAS_THRESHOLD_PPM       1000

/* Sicaklik esigi (1/10 derece). 300 = 30.0 C */
#define TEMP_THRESHOLD_DC       300

/* Olcum periyodu (ms cinsinden, ana dongude ~delay) */
#define READING_PERIOD_MS       2000

/* Sensor isinma suresi (ms) - MQ tipi sensorler icin */
#define WARMUP_PERIOD_MS        3000

#endif /* CONFIG_H */
