#include "app_lab_5_1.h"
#include "app_lab_5_1_task1.h"
#include "app_lab_5_1_task2.h"
#include "app_lab_5_1_task3.h"
#include "task_config.h"
#include "dd_actuator_bin/dd_actuator_bin.h"
#include "dd_actuator_analog/dd_actuator_analog.h"
#include "srv_serial_stdio/srv_serial_stdio.h"
#include "srv_stdio_lcd/srv_stdio_lcd.h"
#include "dd_sns_angle/dd_sns_angle.h"
#include <dd_led/dd_led.h>
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

void app_lab_5_1_setup() {
	srv_serial_stdio_setup();
	srv_stdio_lcd_setup();

	dd_led_setup();
	dd_sns_angle_setup();
	dd_actuator_bin_setup(13, BIN_CMD_PERSISTENCE_SAMPLES);
	dd_actuator_analog_setup(PIN_MOTOR_ENA, PIN_MOTOR_IN1, PIN_MOTOR_IN2);

	dd_led_setup();
	dd_led_turn_off();   // red OFF - binary actuator starts OFF
	dd_led_1_turn_on();  // green ON - system OK at startup
	dd_led_2_turn_off(); // yellow OFF - no alert at startup
	dd_led_apply();
	
	app_lab_5_1_task1_init();
	app_lab_5_1_task2_init();

	xTaskCreate(app_lab_5_1_task1, "Task1_51", 512, NULL, 1, NULL);
	xTaskCreate(app_lab_5_1_task2, "Task2_51", 512, NULL, 3, NULL);
	xTaskCreate(app_lab_5_1_task3, "Task3_51", 768, NULL, 2, NULL);
}

void app_lab_5_1_loop() {
}
