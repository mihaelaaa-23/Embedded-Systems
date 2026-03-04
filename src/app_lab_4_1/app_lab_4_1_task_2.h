#ifndef APP_LAB_4_1_TASK_2_H
#define APP_LAB_4_1_TASK_2_H

#include "task_config.h"

// Task 2 – Signal Conditioning / Threshold Alerting
// Period : 50 ms  (10 ms offset after task_1)
// Priority: 2

void task_2_init();
void task_conditioning(void *pvParameters);

#endif