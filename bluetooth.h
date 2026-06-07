#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <stdint.h>

/* HC-05 Bluetooth modulu icin protokol / paket katmani.
 *
 * Altta uart.c (USCI_A0, 9600 baud) calisir. HC-05 fabrika ayarinda
 * 9600 8N1 olduguna gore ek bir konfigurasyona gerek yok.
 *
 * Donanim:
 *    HC-05 TXD -> MSP P1.1 (UCA0RXD)
 *    HC-05 RXD -> MSP P1.2 (UCA0TXD)
 *    HC-05 VCC -> 5 V
 *    HC-05 GND -> GND
 *
 * Paket bicimi (her 2 sn'de bir, sadece TX acikken):
 *    AQI:<n>,CO:<ppm>,GAS:<ppm>,T:<xx.x>,H:<xx.x>,F:<0|1>\r\n
 */

void bt_init(void);

void bt_send_hello(void);
void bt_send_tx_status(uint8_t on);

void bt_send_packet(uint16_t aqi,
                    uint16_t co_ppm,
                    uint16_t gas_ppm,
                    int16_t  temp_dc,      /* 1/10 derece */
                    uint16_t hum_dp,       /* 1/10 yuzde  */
                    uint8_t  sensors_valid,
                    uint8_t  fan_on);

#endif /* BLUETOOTH_H */
