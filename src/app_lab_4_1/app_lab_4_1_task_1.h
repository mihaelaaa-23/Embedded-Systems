#ifndef APP_LAB_4_1_TASK_1_H
#define APP_LAB_4_1_TASK_1_H

#include <Arduino_FreeRTOS.h>

// Task 1 – Sensor Acquisition
// Period : 50 ms (vTaskDelayUntil)
// Priority: 3 (highest)
void task_acquisition(void *pvParameters);

#endif