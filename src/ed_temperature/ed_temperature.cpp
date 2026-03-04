#include "ed_temperature.h"
#include <Arduino.h>


static int s_raw_value = 0;
static int s_mv_value  = 0;

void ed_temperature_setup() {

}

void ed_temperature_loop() {
    // Citire valoare bruta de la ADC
    s_raw_value = analogRead(ED_TEMPERATURE_ADC_PIN);

    // Conversie RAW -> mV (scalare liniara, Vref = 5000 mV)
    s_mv_value = map(s_raw_value,
                     ED_TEMPERATURE_RAW_MIN,
                     ED_TEMPERATURE_RAW_MAX,
                     ED_TEMPERATURE_MV_MIN,
                     ED_TEMPERATURE_MV_MAX);
}

int ed_temperature_get_raw() {
    return s_raw_value;
}

int ed_temperature_get_mv() {
    return s_mv_value;
}