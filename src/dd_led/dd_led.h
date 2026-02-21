#ifndef DD_LED_H
#define DD_LED_H

#include <Arduino.h>

#define LED_PIN   13
#define LED_1_PIN 12

void dd_led_setup();

// Set target state from ISR (no digitalWrite)
void dd_led_set_target(int val);
void dd_led_1_set_target(int val);

// Apply targets to hardware - call ONLY from idle loop
void dd_led_apply();

// Read current target state (ISR-safe)
int dd_led_is_on();
int dd_led_1_is_on();

// Legacy - only call from main loop
void dd_led_turn_on();
void dd_led_turn_off();
void dd_led_1_turn_on();
void dd_led_1_turn_off();

#endif // DD_LED_H