#ifndef DD_LED_H
#define DD_LED_H

#include <Arduino.h>

void dd_led_setup();
void dd_led_turn_on();
void dd_led_turn_off();

int dd_led_is_on();
#ifndef DD_LED_DEFAULT_PIN
#define DD_LED_DEFAULT_PIN 10
#endif

#endif // DD_LED_H