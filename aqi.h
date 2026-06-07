#ifndef AQI_H
#define AQI_H

#include <stdint.h>

/* Basit AQI (Air Quality Index) hesabi - max yaklasimi.
 *
 *  CO_AQI  = co_ppm  * 500 / AQI_CO_MAX_PPM   (0..500'e kirpilir)
 *  GAS_AQI = gas_ppm * 500 / AQI_GAS_MAX_PPM  (0..500'e kirpilir)
 *  AQI     = max(CO_AQI, GAS_AQI)
 *
 * Anlami: en kotu sensorun normalize edilmis degeri AQI'yi belirler.
 * Tek bir sensorun bile esik asmasi yetiyor. EPA'nin "dominant
 * pollutant" mantigi ile ayni dusunce.
 */

#define AQI_CO_MAX_PPM     200U    /* bu kadar CO  -> AQI 500 */
#define AQI_GAS_MAX_PPM    2000U   /* bu kadar gaz -> AQI 500 */

uint16_t aqi_calc(uint16_t co_ppm, uint16_t gas_ppm);

#endif /* AQI_H */
