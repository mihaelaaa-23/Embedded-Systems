#ifndef DD_SNS_GAS_H
#define DD_SNS_GAS_H

// Gas sensor device driver (MQ-2 / analog) – wraps ed_gas with a FreeRTOS
// mutex so getters are safe to call from any task.
// Maps 0–5000 mV → 0–1000 ppm (linear approximation).

#define DD_SNS_GAS_PPM_MIN    0
#define DD_SNS_GAS_PPM_MAX 1000

void dd_sns_gas_setup();
void dd_sns_gas_loop();       // call from acquisition task (every 50 ms)

// Mutex-protected getters
int  dd_sns_gas_get_raw();      // ADC counts, 0..1023
int  dd_sns_gas_get_voltage();  // millivolts,  0..5000
int  dd_sns_gas_get_ppm();      // gas concentration, 0..1000 ppm

#endif // DD_SNS_GAS_H
