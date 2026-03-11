#include "app_lab_4_2.h"
#include "app_lab_4_2_task_1.h"
#include "app_lab_4_2_task_2.h"
#include "app_lab_4_2_task_3.h"
#include "srv_serial_stdio/srv_serial_stdio.h"
#include "srv_stdio_lcd/srv_stdio_lcd.h"
#include "dd_sns_temperature/dd_sns_temperature.h"
#include "dd_sns_dht/dd_sns_dht.h"
#include "dd_sns_gas/dd_sns_gas.h"
#include "dd_led/dd_led.h"
#include <Arduino_FreeRTOS.h>

void app_lab_4_2_setup() {
    srv_serial_stdio_setup();
    srv_stdio_lcd_setup();
    dd_sns_temperature_setup();
    dd_sns_dht_setup();
    dd_sns_gas_setup();
    dd_led_setup();

    task_4_2_init();

    xTaskCreate(task_acq_4_2,  "Acq42",  512, NULL, 3, NULL);
    xTaskCreate(task_cond_4_2, "Cond42", 512, NULL, 2, NULL);
    xTaskCreate(task_rep_4_2,  "Rep42",  768, NULL, 1, NULL);
}

void app_lab_4_2_loop() {
}
