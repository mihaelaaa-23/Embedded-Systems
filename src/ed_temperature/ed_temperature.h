#ifndef ED_TEMPERATURE_H
#define ED_TEMPERATURE_H


#include <Arduino.h>

// Pin ADC pentru senzorul LM35
#define ED_TEMPERATURE_ADC_PIN   A1

// Limite ADC (rezolutie 10 biti)
#define ED_TEMPERATURE_RAW_MIN   0
#define ED_TEMPERATURE_RAW_MAX   1023

// Limite tensiune (mV), Vref = 5000 mV
#define ED_TEMPERATURE_MV_MIN    0
#define ED_TEMPERATURE_MV_MAX    5000

void ed_temperature_setup();
void ed_temperature_loop();

int ed_temperature_get_raw();
int ed_temperature_get_mv();

#endif // ED_TEMPERATURE_H