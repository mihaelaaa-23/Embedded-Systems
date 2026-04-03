#ifndef APP_LAB_6_1_TASK1_H
#define APP_LAB_6_1_TASK1_H

#include <Arduino_FreeRTOS.h>
#include "task_config.h"

void        app_lab_6_1_task1_init();
void        app_lab_6_1_task1(void *pvParameters);
App61Cmd_t  app_lab_6_1_task1_get_latest();

#endif // APP_LAB_6_1_TASK1_H