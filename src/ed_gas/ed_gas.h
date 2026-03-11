#ifndef ED_GAS_H
#define ED_GAS_H

// Elemental driver for an MQ-2-style gas sensor (analog output).
// In Wokwi the analog output is simulated with a potentiometer on A1.

#define ED_GAS_ADC_PIN A1

#define ED_GAS_RAW_MIN     0
#define ED_GAS_RAW_MAX  1023

#define ED_GAS_VOLTAGE_MIN    0
#define ED_GAS_VOLTAGE_MAX 5000

void ed_gas_setup();
void ed_gas_loop();

int ed_gas_get_raw();       // ADC counts, 0..1023
int ed_gas_get_voltage();   // millivolts,  0..5000

#endif // ED_GAS_H
