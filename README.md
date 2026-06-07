# MSP430G2553 Hava Kalitesi Olcum Projesi

MSP430G2553 tabanli, MQ-7 (CO) + MQ-135 (gaz/CO2) + DHT22 (sicaklik/nem)
sensorleri ile hava kalitesini olcen, **AQI** hesaplayip esik histerezisi
ile fani role uzerinden kontrol eden, verileri HC-05 Bluetooth modulu
uzerinden yayinlayan proje.

## Pin Yerlesimi

| Modul        | MSP430 Pini | Aciklama                                  |
|--------------|-------------|-------------------------------------------|
| DHT22 DATA   | P2.0        | Tek-tel data hatti (4.7k pull-up gerekli) |
| Role IN      | P2.1        | Fan kontrolu (active-high)                |
| MQ-7  AOUT   | P1.4 / A4   | ADC10 analog girisi                       |
| MQ-135 AOUT  | P1.3 / A3   | ADC10 analog girisi                       |
| HC-05 TXD    | P1.1 (RX)   | UART - MSP'nin RXD'si                     |
| HC-05 RXD    | P1.2 (TX)   | UART - MSP'nin TXD'si                     |
| Reset butonu | RST pini    | Her basis BT veri gonderimini toggle eder |

## Dosya Yapisi

```
main.c          Ana dongu: olcum, AQI, fan histerezisi, BT
uart.[ch]       USCI_A0 UART surucusu (9600 8N1 @ 1 MHz)
                !!! Init sirasi (pin -> clock -> UART) DONDURULMUS.
bluetooth.[ch]  HC-05 paket katmani (hello / status / packet)
dht22.[ch]      DHT22 tek-tel surucusu
mq_sensors.[ch] ADC10 ile MQ-7 / MQ-135 ham + ppm okumalari
aqi.[ch]        AQI hesabi (max-of-normalized)
flash.[ch]      Info Segment D'ye TX on/off bayragi (kalici)
relay.[ch]      Role kontrolu (P2.1)
config.h        Esik degerleri ve periyotlar
```

### UART / BT katmanlari neden ayri?

`uart.c` icindeki init sirasi titiz bicimde test edildi:

```
1) P1SEL | P1SEL2  (pin fonksiyonlari)
2) BCSCTL1 = CALBC1_1MHZ; DCOCTL = CALDCO_1MHZ;
3) UCA0CTL1 |= UCSWRST;   (OR, atama degil!)
   ... yapilandirma ...
   UCA0CTL1 &= ~UCSWRST;
```

Bu sira bozulursa BT'ye copluk gider (orn. `@^@^...`). Paket bicimi
degisecekse `bluetooth.c` icinde oynayin, `uart.c` govdesi degismesin.

## AQI ve Fan Mantigi

### AQI hesabi (`aqi.c`)

Her sensor 0..500 araligina normalize edilir; en kotusu AQI olur:

```
co_aqi  = co_ppm  * 500 / 200    (kirpilir 0..500)
gas_aqi = gas_ppm * 500 / 2000   (kirpilir 0..500)
AQI     = max(co_aqi, gas_aqi)
```

Yani bir sensor "iyi" diger "kotu" olsa bile AQI kotuyu yansitir.

### Fan histerezisi (`config.h`)

| Durum            | Davranis                             |
|------------------|--------------------------------------|
| AQI > 150 (ON)   | Fan acilir                           |
| AQI < 100 (OFF)  | Fan kapanir                          |
| 100 .. 150 arasi | Mevcut durum korunur (titremeyi onler)|

Ekstra: `T > 30.0 C` ise fan AQI'den bagimsiz acilir (override).
Sogudugunda yine AQI karari verir.

### Esikleri degistirme

`config.h` icinde:

| Sabit                    | Varsayilan |
|--------------------------|------------|
| AQI_FAN_ON_THRESHOLD     | 150        |
| AQI_FAN_OFF_THRESHOLD    | 100        |
| TEMP_FAN_ON_DC           | 30.0 C     |
| READING_PERIOD_MS        | 2000 ms    |
| WARMUP_PERIOD_MS         | 3000 ms    |

## RST Butonu - BT Toggle

MSP430'nin RST pini fiziksel butonla yere cekiliyor. Her basis:

1. Cipi resetler.
2. Boot anlaminda `main()` calisir.
3. `flash_toggle_tx_flag()` Info Segment D'deki bayragi cevirir
   ve flash'a yazar.
4. Yeni durum `tx_enable` degiskenine alinir; donguye girilir.

Sonuc:

- **TX ON**:  her 2 sn'de BT'ye paket gider, baslangicta `[INFO] TX ON`.
- **TX OFF**: BT'ye HICBIR sey gitmez (`bt_send_*` cagrilari atlanir),
  ama sensor okuma + fan kontrolu calismaya devam eder.

Durum **kalicidir** — guc kesilse de korunur. Cip ilk programlandiktan
sonraki ilk acilis varsayilan olarak **TX ON**'dur.

## Bluetooth Veri Bicimi

```
AQI:124,CO:42,GAS:780,T:24.5,H:51.2,F:0
```

| Alan | Aciklama                                         |
|------|--------------------------------------------------|
| AQI  | 0..500, max(CO_norm, GAS_norm)                   |
| CO   | MQ-7 ppm (lineer yaklasim)                       |
| GAS  | MQ-135 ppm (lineer yaklasim)                     |
| T    | DHT22 sicaklik, 1 ondalik. Okuma hatasi: `ERR`   |
| H    | DHT22 nem, 1 ondalik. Okuma hatasi: `ERR`        |
| F    | Fan durumu (0 / 1)                               |

## Derleme

Code Composer Studio'da yeni bir **MSP430G2553** projesi olusturup
tum `.c` ve `.h` dosyalarini proje koklerine ekleyin. Ek bir
kutuphaneye gerek yoktur.

**CCS Flash ayari:** "Erase Main memory only" tutun (Project Properties
-> Debug -> MSP430 Properties -> Erase Options). Boylece info
segment'lerdeki TX bayragi yeniden programlama sirasinda silinmez.
Tam silmek isterseniz "Erase Main and Information memory" secebilirsiniz.

## Kalibrasyon

MQ sensorler RL ve Ro'ya bagli log10 egri ile gercek ppm verir.
Su anki kod **lineer yaklasim** kullaniyor:

- MQ-7   : 0..1023 ADC -> 0..1000 ppm CO
- MQ-135 : 0..1023 ADC -> 0..2000 ppm gaz

Hassas olcum gerekiyorsa `mq_sensors.c` icindeki donusumler log10
formulu ile degistirilir; AQI normalizasyon ust sinirlari da
`aqi.h` icinden yeniden ayarlanir.

## Donanim Notlari

1. **DHT22**: data hattina 4.7k - 10k pull-up.
2. **MQ sensorler**: ilk acilista 24-48 saat preheat onerilir.
3. **HC-05**: 9600 8N1 varsayildi. Farkliysa `uart.c` icinde
   `UCA0BR0` ve `UCA0MCTL` guncellenmeli.
4. **Role karti** active-low ise `relay.h` icinde
   `RELAY_ACTIVE_LOW` 1 yapilmali.
5. **Clock**: `bt_init()` DCO'yu 1 MHz'e ceker; tum zamanlamalar
   (DHT22, delay_ms, ADC, flash timing) buna gore yazildi.
   `bt_init()` HER ZAMAN ILK cagrilmalidir.
