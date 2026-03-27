#include "app_lab_5_2.h"
#include "app_lab_5_2_task1.h"
#include "app_lab_5_2_task2.h"
#include "app_lab_5_2_task3.h"
#include "task_config.h"
#include "dd_actuator_analog/dd_actuator_analog.h"
#include "dd_actuator_bin/dd_actuator_bin.h"
#include "dd_led/dd_led.h"
#include "ed_potentiometer/ed_potentiometer.h"
#include "srv_serial_stdio/srv_serial_stdio.h"
#include "srv_stdio_lcd/srv_stdio_lcd.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

void app_lab_5_2_setup() {
    srv_serial_stdio_setup();
    srv_stdio_lcd_setup();

    // Setup binary actuator (relay/solenoid)
    dd_actuator_bin_setup(PIN_BIN_ACT, BIN_PERSISTENCE_SAMPLES);
    
    // Setup analog actuator (motor)
    dd_actuator_analog_setup(PIN_MOTOR_ENA, PIN_MOTOR_IN1, PIN_MOTOR_IN2);
    
    // Setup potentiometer for AUTO mode
    ed_potentiometer_setup();

    dd_led_setup();
    dd_led_turn_off();    // red OFF
    dd_led_1_turn_on();   // green ON - system OK
    dd_led_2_turn_off();  // yellow OFF
    dd_led_apply();

    app_lab_5_2_task1_init();
    app_lab_5_2_task2_init();

    // Task 1: lowest priority - blocks on scanf
    xTaskCreate(app_lab_5_2_task1, "Task1_52", 512, NULL, 1, NULL);
    // Task 2: highest priority - binary + analog control
    xTaskCreate(app_lab_5_2_task2, "Task2_52", 512, NULL, 3, NULL);
    // Task 3: mid priority - display
    xTaskCreate(app_lab_5_2_task3, "Task3_52", 768, NULL, 2, NULL);
}

void app_lab_5_2_loop() {}