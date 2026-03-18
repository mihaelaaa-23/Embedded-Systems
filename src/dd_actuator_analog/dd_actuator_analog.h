#ifndef DD_ACTUATOR_ANALOG_H
#define DD_ACTUATOR_ANALOG_H

#include <Arduino.h>

#define DD_ACTUATOR_ANALOG_PWM_MIN 0
#define DD_ACTUATOR_ANALOG_PWM_MAX 255

void dd_actuator_analog_setup(uint8_t ena_pin, uint8_t in1_pin, uint8_t in2_pin);
void dd_actuator_analog_set_requested_pwm(int pwm);
void dd_actuator_analog_apply();

int dd_actuator_analog_get_requested_pwm();
int dd_actuator_analog_get_applied_pwm();

#endif // DD_ACTUATOR_ANALOG_H
