#ifndef APP_LAB_3_2_TASK_2_H
#define APP_LAB_3_2_TASK_2_H

#include "Arduino_FreeRTOS.h"
#include "semphr.h"

extern SemaphoreHandle_t xSemaphore;
extern SemaphoreHandle_t xStatsMutex;

extern int           g_3_2_total_presses;
extern int           g_3_2_short_presses;
extern int           g_3_2_long_presses;
extern unsigned long g_3_2_sum_short_ms;
extern unsigned long g_3_2_sum_long_ms;

void app_lab_3_2_task_2(void *pvParameters);

#endif