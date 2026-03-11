#ifndef APP_LAB_4_2_TASK_1_H
#define APP_LAB_4_2_TASK_1_H

#include <Arduino_FreeRTOS.h>

// Task 1 – Sensor Acquisition
// Reads S1 (analog potentiometer) and S2 (DHT22) at ACQ_PERIOD_MS.
// Priority: 3 (highest)
void task_acq_4_2(void *pvParameters);

#endif // APP_LAB_4_2_TASK_1_H