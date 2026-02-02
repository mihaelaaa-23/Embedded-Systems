#ifndef DD_LED_H
#define DD_LED_H

#include <Arduino.h>

#define LED_PIN LED_BUILTIN

void dd_led_setup();
void dd_led_turn_on();
void dd_led_turn_off();

int dd_led_is_on();

#endif // DD_LED_H