#ifndef DD_ACTUATOR_BIN_H
#define DD_ACTUATOR_BIN_H

#include <Arduino.h>

void dd_actuator_bin_setup(uint8_t pin, uint8_t persistence_samples);
void dd_actuator_bin_set_requested(bool on);
void dd_actuator_bin_step();

bool dd_actuator_bin_get_requested();
bool dd_actuator_bin_get_pending();
bool dd_actuator_bin_get_state();

#endif // DD_ACTUATOR_BIN_H
