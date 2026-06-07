#include "aqi.h"

static uint16_t scale_to_aqi(uint32_t value, uint32_t max_value)
{
    uint32_t aqi;
    if (value >= max_value) return 500U;
    aqi = (value * 500U) / max_value;
    if (aqi > 500U) aqi = 500U;
    return (uint16_t)aqi;
}

uint16_t aqi_calc(uint16_t co_ppm, uint16_t gas_ppm)
{
    uint16_t a = scale_to_aqi(co_ppm,  AQI_CO_MAX_PPM);
    uint16_t b = scale_to_aqi(gas_ppm, AQI_GAS_MAX_PPM);
    return (a > b) ? a : b;
}
