#include "dd_led.h"

void dd_led_setup() {
    pinMode(DD_LED_DEFAULT_PIN, OUTPUT);
    dd_led_turn_off();
}

void dd_led_turn_on() {
    digitalWrite(DD_LED_DEFAULT_PIN, HIGH);
}

void dd_led_turn_off() {
    digitalWrite(DD_LED_DEFAULT_PIN, LOW);
}

int dd_led_is_on() {
    return (digitalRead(DD_LED_DEFAULT_PIN) == HIGH) ? 1 : 0;
}