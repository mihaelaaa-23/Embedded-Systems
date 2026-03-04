#ifndef ED_DHT_H
#define ED_DHT_H

#include <Arduino.h>

// Digital pin connected to DHT22 data line
#define ED_DHT_PIN  2

// Minimum interval between real sensor reads (DHT22 spec: 2 s)
#define ED_DHT_MIN_INTERVAL_MS  2000

void ed_dht_setup();
void ed_dht_loop();          // call from acquisition task; throttled internally

int  ed_dht_get_raw();       // temperature × 10  (0.1 °C resolution)
int  ed_dht_get_celsius();   // integer °C
int  ed_dht_get_humidity();  // integer %RH

#endif // ED_DHT_H
