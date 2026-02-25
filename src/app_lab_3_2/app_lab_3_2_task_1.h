#ifndef APP_LAB_3_2_TASK_1_H
#define APP_LAB_3_2_TASK_1_H

#include "Arduino_FreeRTOS.h"
#include "semphr.h"

#define TASK_1_REC          10
#define TASK_1_OFFSET       100
#define PRESS_THRESHOLD_MS  500
#define DEBOUNCE_TICKS      3
#define MIN_PRESS_MS        30
#define MAX_PRESS_MS        5000

extern SemaphoreHandle_t xSemaphore;
extern SemaphoreHandle_t xStatsMutex;

extern int g_3_2_last_duration_ms;
extern int g_3_2_last_was_short;

void app_lab_3_2_task_1(void *pvParameters);

#endif