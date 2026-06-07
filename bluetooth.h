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
 *    HC-05 VCC -> 5 V (modul kartinin 5 V regulator girisinden)
 *    HC-05 GND -> GND ortak
 *
 * Paket bicimi (her 2 sn'de bir):
 *    CO:<ppm>,GAS:<ppm>,T:<xx.x>,H:<xx.x>,F:<0|1>\r\n
 */

void bt_init(void);

/* Acilista bir kez "alive" mesaji - terminalde modulun gercekten
 * UART bagli ve baud'larin tuttugundan emin olmak icin.            */
void bt_send_hello(void);

/* Tam paket. sensors_valid = 0 ise T/H yerine "ERR" yazilir,
 * CO/GAS hala basilir (ADC her zaman calisir).                    */
void bt_send_packet(uint16_t co_ppm,
                    uint16_t gas_ppm,
                    int16_t  temp_dc,      /* 1/10 derece */
                    uint16_t hum_dp,       /* 1/10 yuzde  */
                    uint8_t  sensors_valid,
                    uint8_t  fan_on);

#endif /* BLUETOOTH_H */
