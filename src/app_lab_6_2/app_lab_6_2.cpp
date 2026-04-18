#include "app_lab_6_2.h"
#include "app_lab_6_2_task1.h"
#include "app_lab_6_2_task2.h"
#include "app_lab_6_2_task3.h"
#include "task_config.h"
#include "../dd_actuator_bin/dd_actuator_bin.h"
#include "../dd_led/dd_led.h"
#include "../dd_sns_dht/dd_sns_dht.h"
#include "../srv_serial_stdio/srv_serial_stdio.h"
#include "../srv_stdio_lcd/srv_stdio_lcd.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

void app_lab_6_2_setup() {
    srv_serial_stdio_setup();
    srv_stdio_lcd_setup();
    dd_sns_dht_setup();
    dd_actuator_bin_setup(PIN_RELAY, 1);
    dd_led_setup();
    dd_led_turn_off();
    dd_led_1_turn_on();
    dd_led_2_turn_off();
    dd_led_apply();
    app_lab_6_2_task1_init();
    app_lab_6_2_task2_init();
    xTaskCreate(app_lab_6_2_task1, "T1_62", 512, NULL, 1, NULL);
    xTaskCreate(app_lab_6_2_task2, "T2_62", 512, NULL, 3, NULL);
    xTaskCreate(app_lab_6_2_task3, "T3_62", 768, NULL, 2, NULL);
}

void app_lab_6_2_loop() {}
