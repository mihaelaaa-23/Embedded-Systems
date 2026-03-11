#ifndef APP_LAB_4_2_TASK_3_H
#define APP_LAB_4_2_TASK_3_H

#include <Arduino_FreeRTOS.h>

// Task 3 – Display & Reporting
// Prints structured report (raw / saturated / median / WMA / alert) every
// REPORT_PERIOD_MS ms via stdout (teed to LCD + Serial).
// Priority: 1 (lowest)
void task_rep_4_2(void *pvParameters);

#endif // APP_LAB_4_2_TASK_3_H