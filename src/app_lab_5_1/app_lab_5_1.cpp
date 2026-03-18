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
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

void app_lab_5_1_setup() {
	srv_serial_stdio_setup();
	srv_stdio_lcd_setup();

	dd_sns_angle_setup();
	dd_actuator_bin_setup(13, BIN_CMD_PERSISTENCE_SAMPLES);
	dd_actuator_analog_setup(PIN_MOTOR_ENA, PIN_MOTOR_IN1, PIN_MOTOR_IN2);

	pinMode(PIN_LED_BIN_ON, OUTPUT);  // red: binary actuator ON
	digitalWrite(PIN_LED_BIN_ON, LOW);
	pinMode(PIN_LED_OK, OUTPUT);      // green: system OK / no analog alert
	pinMode(PIN_LED_ALERT, OUTPUT);   // yellow: analog alert active
	digitalWrite(PIN_LED_OK, HIGH);
	digitalWrite(PIN_LED_ALERT, LOW);

	app_lab_5_1_task1_init();
	app_lab_5_1_task2_init();

	xTaskCreate(app_lab_5_1_task1, "Task1_51", 512, NULL, 3, NULL);
	xTaskCreate(app_lab_5_1_task2, "Task2_51", 512, NULL, 2, NULL);
	xTaskCreate(app_lab_5_1_task3, "Task3_51", 768, NULL, 1, NULL);
}

void app_lab_5_1_loop() {
}
