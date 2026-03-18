#ifndef APP_LAB_5_1_TASK1_H
#define APP_LAB_5_1_TASK1_H

#include <Arduino_FreeRTOS.h>
#include "task_config.h"

typedef struct {
    bool bin_requested;
    AnalogControlMode analog_mode;
    int manual_pwm;
} App5UserCmd_t;

void app_lab_5_1_task1_init();
void app_lab_5_1_task1(void *pvParameters);

App5UserCmd_t app_lab_5_1_task1_get_latest();

#endif // APP_LAB_5_1_TASK1_H
