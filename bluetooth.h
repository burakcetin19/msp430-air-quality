#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <msp430.h>
#include <stdint.h>

/* HC-05 pinleri:
 *   P1.1 = RXD (MSP UART RX)
 *   P1.2 = TXD (MSP UART TX)
 */
#define BT_RX   BIT1
#define BT_TX   BIT2

void bt_init(void);
void bt_send_char(char c);
void bt_send_string(const char *str);
void bt_send_number(uint16_t num);
void bt_send_signed(int16_t num);

#endif /* BLUETOOTH_H */
