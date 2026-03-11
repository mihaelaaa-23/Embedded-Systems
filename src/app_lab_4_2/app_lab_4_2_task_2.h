#ifndef APP_LAB_4_2_TASK_2_H
#define APP_LAB_4_2_TASK_2_H

#include "task_config.h"

// Task 2 – Signal Conditioning
// Pipeline: saturation → 5-point median filter → IIR weighted moving average
// Period: ACQ_PERIOD_MS (10 ms offset after task_1)
// Priority: 2

void task_4_2_init();
void task_cond_4_2(void *pvParameters);

#endif // APP_LAB_4_2_TASK_2_H