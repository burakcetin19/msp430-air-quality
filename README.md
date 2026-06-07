# MSP430G2553 Hava Kalitesi Olcum Projesi

MSP430G2553 tabanli, MQ-7 (CO) + MQ-135 (gaz/CO2) + DHT22 (sicaklik/nem)
sensorleri ile hava kalitesini olcen, esik asildiginda role uzerinden fani
calistiran ve HC-05 Bluetooth modulu uzerinden verileri yayinlayan proje.

## Pin Yerlesimi

| Modul        | MSP430 Pini | Aciklama                         |
|--------------|-------------|----------------------------------|
| DHT22 DATA   | P2.0        | Tek-tel data hatti (4.7k pull-up)|
| Role IN      | P2.3        | Fan kontrolu (active-high)       |
| MQ-7  AOUT   | P1.4 / A4   | ADC10 analog girisi              |
| MQ-135 AOUT  | P1.3 / A3   | ADC10 analog girisi              |
| HC-05 TXD    | P1.1 (RX)   | UART (MSP RX)                    |
| HC-05 RXD    | P1.2 (TX)   | UART (MSP TX) - 3.3V seviye      |
| RST          | RST pini    | Sifirlama butonu                 |

> HC-05'in RXD'si 3.3V toleransli degildir. MSP TX 3.3V cikis verir,
> bu yon sorunsuzdur. Ancak HC-05 -> MSP yonunde modul 3.3V CIKIS verdigi
> icin direkt baglanabilir.

## Dosya Yapisi

```
main.c          Ana dongu: olcum, esik, role, BT cagrisi
uart.[ch]       USCI_A0 UART surucusu (9600 8N1 @ 1 MHz)
                !!! Init sirasi (pin -> clock -> UART) DONDURULMUS,
                    bu dosyaya dokunma. !!!
bluetooth.[ch]  HC-05 paket katmani: bt_init / hello / send_packet
dht22.[ch]      DHT22 tek-tel surucusu (1 MHz MCLK)
mq_sensors.[ch] ADC10 ile MQ-7 / MQ-135 okumalari
relay.[ch]      Role kontrolu (P2.3)
config.h        Esik degerleri ve periyotlar
```

### UART / BT katmanlari neden ayri?

`uart.c` icindeki init sirasi titiz biçimde test edildi
(pin fonksiyonu -> DCO kalibrasyonu -> `UCSWRST` altinda `|=` ile
yapilandirma). Bu sira bozulursa BT'ye copluk (orn. `@^@^...`) gider.
Paket bicimi degisecekse `bluetooth.c` icinde oynayin, `uart.c`
govdesi degismesin.

## Esik Degerleri

`config.h` dosyasindan degistirilebilir:

| Esik                | Varsayilan  |
|---------------------|-------------|
| CO_THRESHOLD_PPM    | 50 ppm      |
| GAS_THRESHOLD_PPM   | 1000 ppm    |
| TEMP_THRESHOLD_DC   | 30.0 C      |
| READING_PERIOD_MS   | 2000 ms     |
| WARMUP_PERIOD_MS    | 3000 ms     |

Esiklerden HERHANGI BIRI asilirsa fan (role) AC.

## Bluetooth Veri Bicimi

Her 2 saniyede bir su satir gonderilir:

```
CO:42,GAS:780,T:24.5,H:51.2,F:0
```

- `CO`   : MQ-7 ppm (lineer yaklasim)
- `GAS`  : MQ-135 ppm (lineer yaklasim)
- `T`    : DHT22 sicaklik, 1 ondalik
- `H`    : DHT22 nem, 1 ondalik
- `F`    : Fan durumu (0 = kapali, 1 = acik)

DHT22 okumasi basarisiz olursa `T:ERR,H:ERR` doner; fan karari son
basarili olcume gore alinir.

## Derleme

Code Composer Studio veya Energia / mspgcc ile:

CCS'de yeni bir MSP430G2553 projesi olusturup tum `.c` ve `.h` dosyalarini
proje koklerine ekleyin. Ek bir kutuphaneye gerek yoktur.

## Kalibrasyon Notu

MQ-7 ve MQ-135 cikislari, RL yuk direnci ve sensor Ro degerine bagli
LOGARITMIK bir egri ile gercek ppm'e cevrilir. Bu projede esik kararinin
verilebilmesi icin **lineer yaklasim** kullanilmistir:

- MQ-7   : 0..1023 ADC -> 0..1000 ppm CO
- MQ-135 : 0..1023 ADC -> 0..2000 ppm gaz

Daha hassas olcum istenirse `mq_sensors.c` icindeki donusum
fonksiyonlari log10 tabanli formul ile degistirilmelidir.

## Donanim Notlari

1. **DHT22**: data hattina 4.7k - 10k pull-up direnci konulmali.
2. **MQ sensorler**: ilk acilista 24-48 saat preheat onerilir. Her
   acilista da en az 3 sn (`WARMUP_PERIOD_MS`) isinma vardir.
3. **HC-05**: varsayilan baud 9600. AT moduyla degistirilmissse
   `bluetooth.c` icindeki `UCA0BR0` degerini guncelleyin.
4. **Role karti**: active-low ise `relay.h` icinde
   `RELAY_ACTIVE_LOW` 1 yapilmali.
5. **Clock**: bt_init() DCO'yu 1 MHz'e cektigi icin tum zamanlamalar
   (DHT22, delay_ms, ADC) 1 MHz varsayar. `bt_init()` ilk cagrilmalidir.
