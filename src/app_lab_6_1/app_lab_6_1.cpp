#include "app_lab_6_1.h"
#include "app_lab_6_1_task1.h"
#include "app_lab_6_1_task2.h"
#include "app_lab_6_1_task3.h"
#include "task_config.h"
#include "dd_actuator_bin/dd_actuator_bin.h"
#include "dd_led/dd_led.h"
#include "dd_sns_dht/dd_sns_dht.h"
#include "srv_serial_stdio/srv_serial_stdio.h"
#include "srv_stdio_lcd/srv_stdio_lcd.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

void app_lab_6_1_setup() {
    srv_serial_stdio_setup();
    srv_stdio_lcd_setup();

    // DHT11 sensor (pin 2, defined inside ed_dht.h)
    dd_sns_dht_setup();

    // Relay on PIN_RELAY with persistence debounce
    dd_actuator_bin_setup(PIN_RELAY, BIN_PERSISTENCE_SAMPLES);

    // LEDs
    dd_led_setup();
    dd_led_turn_off();    // red OFF
    dd_led_1_turn_on();   // green ON  - system OK at startup
    dd_led_2_turn_off();  // yellow OFF
    dd_led_apply();

    // Init shared state
    app_lab_6_1_task1_init();
    app_lab_6_1_task2_init();

    // Task 1 – priority 1 (lowest): blocks on scanf for UP/DOWN/SP/HYST commands
    xTaskCreate(app_lab_6_1_task1, "Task1_61", 512, NULL, 1, NULL);

    // Task 2 – priority 3 (highest): acquire sensor, ON-OFF hysteresis control, relay
    xTaskCreate(app_lab_6_1_task2, "Task2_61", 512, NULL, 3, NULL);

    // Task 3 – priority 2 (mid): LCD + serial reporting
    xTaskCreate(app_lab_6_1_task3, "Task3_61", 768, NULL, 2, NULL);
}

void app_lab_6_1_loop() {
    // FreeRTOS scheduler owns the CPU; nothing here
}