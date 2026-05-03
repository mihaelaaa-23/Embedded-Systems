#include "app_lab_7_1.h"
#include "app_lab_7_1_task1.h"
#include "app_lab_7_1_task2.h"
#include "app_lab_7_1_task3.h"
#include "task_config.h"
#include "../dd_button/dd_button.h"
#include "../dd_led/dd_led.h"
#include "../srv_serial_stdio/srv_serial_stdio.h"
#include "../srv_stdio_lcd/srv_stdio_lcd.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
// Note: dd_actuator_bin is NOT used here — the FSM debounce is implemented
// directly in task2 to avoid driving an INPUT_PULLUP pin as OUTPUT.

void app_lab_7_1_setup() {
    srv_serial_stdio_setup();
    srv_stdio_lcd_setup();

    dd_button_setup();

    dd_led_setup();
    dd_led_turn_off();   // pin 9  (red)   – LED starts OFF
    dd_led_1_turn_on();  // pin 12 (green) – system OK
    dd_led_2_turn_off(); // pin 11 (yellow)– mirrors FSM, starts OFF
    dd_led_apply();

    app_lab_7_1_task2_init();

    // T1: Acquisition  – highest priority, fastest period
    xTaskCreate(app_lab_7_1_task1, "T1_71", 256, NULL, 3, NULL);
    // T2: FSM+Cond     – medium priority
    xTaskCreate(app_lab_7_1_task2, "T2_71", 512, NULL, 2, NULL);
    // T3: Display      – lowest priority, slowest period
    xTaskCreate(app_lab_7_1_task3, "T3_71", 768, NULL, 1, NULL);
}

void app_lab_7_1_loop() {}