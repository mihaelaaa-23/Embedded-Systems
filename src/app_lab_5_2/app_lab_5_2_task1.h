#ifndef APP_LAB_5_2_TASK1_H
#define APP_LAB_5_2_TASK1_H

#include <Arduino_FreeRTOS.h>
#include "task_config.h"

// User command: just a raw PWM target (0..255)
typedef struct {
    int  target_pwm;   // desired PWM value
} App52Cmd_t;

void          app_lab_5_2_task1_init();
void          app_lab_5_2_task1(void *pvParameters);
App52Cmd_t    app_lab_5_2_task1_get_latest();

#endif