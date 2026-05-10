#include "app_lab_7_2.h"
#include "app_lab_7_2_task1.h"
#include "app_lab_7_2_task2.h"
#include "app_lab_7_2_task3.h"
#include "task_config.h"
#include "../dd_button/dd_button.h"
#include "../dd_led/dd_led.h"
#include "../srv_serial_stdio/srv_serial_stdio.h"
#include "../srv_stdio_lcd/srv_stdio_lcd.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

void app_lab_7_2_setup() {
    srv_serial_stdio_setup();
    srv_stdio_lcd_setup();

    // Both buttons (NS request + emergency) — INPUT_PULLUP via dd_button
    dd_button_setup();

    // EW LEDs via dd_led (pins 9, 11, 12)
    dd_led_setup();

    // NS LEDs — direct pinMode (pins 6, 7, 8)
    pinMode(PIN_NS_RED,    OUTPUT);
    pinMode(PIN_NS_YELLOW, OUTPUT);
    pinMode(PIN_NS_GREEN,  OUTPUT);

    // Initial state: EW green, NS red
    dd_led_turn_off();      // pin 9  EW red   OFF
    dd_led_2_turn_off();    // pin 11 EW yellow OFF
    dd_led_1_turn_on();     // pin 12 EW green  ON
    dd_led_apply();

    digitalWrite(PIN_NS_RED,    HIGH);  // NS red ON
    digitalWrite(PIN_NS_YELLOW, LOW);
    digitalWrite(PIN_NS_GREEN,  LOW);

    app_lab_7_2_task2_init();

    // T1: Input acquisition — highest priority, fastest period
    xTaskCreate(app_lab_7_2_task1, "T1_72", 256, NULL, 3, NULL);
    // T2: FSM evaluation — medium priority
    xTaskCreate(app_lab_7_2_task2, "T2_72", 512, NULL, 2, NULL);
    // T3: Display — lowest priority, event-driven
    xTaskCreate(app_lab_7_2_task3, "T3_72", 768, NULL, 1, NULL);
}

void app_lab_7_2_loop() {}