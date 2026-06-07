#ifndef AQI_H
#define AQI_H

#include <stdint.h>

/* Basit AQI: iki sensorun normalize edilmis halinin maksimumu.
 *   CO  0..AQI_CO_MAX_PPM   -> 0..500
 *   GAS 0..AQI_GAS_MAX_PPM  -> 0..500
 *   AQI = max(CO_norm, GAS_norm)
 *
 * Sensor lineer (mq_sensors.c'deki ham->ppm donusumu) oldugu icin
 * sonuc da lineer; sadece esik karari icin kullaniliyor.
 */

#define AQI_CO_MAX_PPM     200U    /* bu kadar CO  -> AQI 500 */
#define AQI_GAS_MAX_PPM    2000U   /* bu kadar gaz -> AQI 500 */

uint16_t aqi_calc(uint16_t co_ppm, uint16_t gas_ppm);

#endif /* AQI_H */
